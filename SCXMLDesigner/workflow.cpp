#include <QDebug>
#include <QGraphicsRectItem>
#include "workflow.h"
#include "scxmlstate.h"
#include "scxmltransition.h"
#include "utilities.h"
#include "xmlutilities.h"
#include "scxmlexecutablecontent.h"

Workflow::Workflow() :
    QStateMachine()
{
}

void Workflow::ConstructSCXMLFromStateMachine(QDomDocument &doc)
{
    // ensure we have no existing content
    doc.clear();

    // create the root element with name attribute
    QDomElement rootElement = doc.createElementNS("http://www.w3.org/2005/07/scxml", "scxml");
    rootElement.setAttribute("name", mName);
    rootElement.setAttribute("version", "1.0");
    doc.appendChild(rootElement);

    // data model
    if (mDataModel.HasItems()) {
        QDomElement dataModelElement = doc.createElement("datamodel");
        rootElement.appendChild(dataModelElement);
        foreach (SCXMLDataItem* dataItem, mDataModel.GetDataItemList()) {
            QDomElement dataElement = doc.createElement("data");
            dataElement.setAttribute("id", dataItem->GetId());
            if (dataItem->GetSrc() != "") {
                dataElement.setAttribute("src", dataItem->GetSrc());
            }
            if (dataItem->GetExpr() != "") {
                dataElement.setAttribute("expr", dataItem->GetExpr());
            }
            dataModelElement.appendChild(dataElement);
        }
    }

    // traverse the states to build up the SCXML document
    foreach(QObject* child, this->children()) {
        SCXMLState* state = dynamic_cast<SCXMLState*>(child);

        QDomElement element = doc.createElement(state->GetFinal() ? "final" : "state");
        element.setAttribute("id", state->GetId());

        // add the state meta-data comment
        QDomComment metaDataComment = doc.createComment(state->GetMetaDataString());
        element.appendChild(metaDataComment);

        // add the onentry
//        QDomElement onEntryElement = state->GetOnEntry()->ToXMLElement(doc);
//        element.appendChild(onEntryElement);

        // add the transitions
        foreach(QAbstractTransition* trans, state->transitions()) {
            SCXMLTransition* transition = dynamic_cast<SCXMLTransition*>(trans);
            QDomElement transitionElement = doc.createElement("transition");
            transitionElement.setAttribute("type", transition->getTransitionType());
            SCXMLState* targetState = dynamic_cast<SCXMLState*>(transition->targetState());
            transitionElement.setAttribute("target", targetState->GetId());
            QString event = transition->GetEvent();
            if (!event.isEmpty()) {
                transitionElement.setAttribute("event", event);
            }
            transitionElement.setAttribute("target", targetState->GetId());

            // add the transition meta-data comment
            QDomComment metaDataComment = doc.createComment(transition->GetMetaDataString());
            transitionElement.appendChild(metaDataComment);

            element.appendChild(transitionElement);
        }
        rootElement.appendChild(element);
    }
}

void Workflow::ConstructStateMachineFromSCXML(QDomDocument &doc)
{
    mRawSCXMLText = doc.toString();

    // ensure we have no existing state machine
    foreach(QObject* child, this->children()) {
        SCXMLState* state = dynamic_cast<SCXMLState*>(child);
        removeState(state);
    }

    // traverse the SCXML to build up the state machine
    QDomNodeList scxmlElements = doc.elementsByTagName("scxml");
    if (scxmlElements.length() != 1) {
        Utilities::ShowWarning("SCXML file does not have a single scxml tag");
        return;
    }

    // get the name of the workflow
    QDomElement scxmlRoot = scxmlElements.at(0).toElement();
    mName = scxmlRoot.attribute("name", "Unnamed");

    // add all the states before we add transitions (they need to exist!)
    QStringList stateTags;
    stateTags << "state" << "final";
    QList<QDomNode> allElements;
    XMLUtilities::GetElementsWithTagNames(allElements, doc, stateTags, true);
    for (int elementPos=0; elementPos<allElements.length(); elementPos++) {
        QDomElement element = allElements.at(elementPos).toElement();
        QString id = element.attribute("id", "unnamed");
//        SCXMLExecutableContent *onEntryContent = nullptr;
//        QDomNodeList onEntryElements = element.elementsByTagName("onentry");
//        if (onEntryElements.count() > 0) {
//            onEntryContent = SCXMLOnEntry::FromXmlElement(onEntryElements.at(0));
//        }

        QMap<QString,QString> metaData = ExtractMetaDataFromElementComments(&element);
        SCXMLState *newState = new SCXMLState(id, &metaData);
        ExtractDataModelFromElement(&element, newState);
//        if (onEntryElements.count() > 0) {
//            QString str;
//            QTextStream stream(&str);
//            QDomElement onEntryElement = onEntryElements.at(0).toElement();
//            onEntryElement.save(stream, QDomNode::EncodingFromDocument);
//            newState->SetOnEntry(str);
//        }
        //newState->SetOnEntry(onEntryContent);
        addState(newState);
    }

    // add top level data model if exists
    ExtractDataModelFromElement(&scxmlRoot, nullptr);

    // add transitions
    for (int elementPos=0; elementPos<allElements.length(); elementPos++) {
        QDomElement element = allElements.at(elementPos).toElement();
        QString id = element.attribute("id", "unnamed");
        SCXMLState *sourceState = GetStateById(id);
        QDomNodeList stateTransitions = element.elementsByTagName("transition");
        for (int transitionPos=0; transitionPos<stateTransitions.length(); transitionPos++) {
            QDomElement stateTransition = stateTransitions.at(transitionPos).toElement();
            QString transitionTarget = stateTransition.attribute("target", "");
            QString transitionType = stateTransition.attribute("type", "");
            QString transitionEvent = stateTransition.attribute("event", "");

            SCXMLState* targetState = GetStateById(transitionTarget);
            if (targetState == nullptr) {
                qDebug() << "No such state: " << transitionTarget;
                continue;
            }
            QMap<QString,QString> metaData = ExtractMetaDataFromElementComments(&stateTransition);
            SCXMLTransition* newTransition = new SCXMLTransition(sourceState, targetState, transitionEvent, transitionType, &metaData);
        }

        // need to adjust start and end points with update
        sourceState->UpdateTransitions();

        //FIXME: implement this
        //setInitialState(newState);
    }
}

void Workflow::ExtractDataModelFromElement(QDomElement* element, SCXMLState* state)
{
    QDomNodeList allSubNodes = element->childNodes();
    for (int subPos=0; subPos<allSubNodes.length(); subPos++) {
        QDomNode node = allSubNodes.at(subPos);
        if (!node.isElement()) continue;
        QDomElement elementDataModel = node.toElement();
        if (elementDataModel.nodeName() != "datamodel") continue;

        // found the data model, now traverse the data items
        QDomNodeList dataNodes = elementDataModel.elementsByTagName("data");
        for (int dataPos=0; dataPos<dataNodes.length(); dataPos++) {
            QDomNamedNodeMap attrMap = dataNodes.at(dataPos).attributes();
            QString src = "";
            QString expr = "";
            if (attrMap.contains("src")) src = attrMap.namedItem("src").toAttr().value();
            if (attrMap.contains("expr")) expr = attrMap.namedItem("expr").toAttr().value();
            if (attrMap.contains("id")) {
                SCXMLDataItem* dataItem = new SCXMLDataItem(attrMap.namedItem("id").toAttr().value(), src, expr);
                mDataModel.AddDataItem(dataItem);
                if (state != nullptr) {
                    //TODO: add details to state
                }
            }
        }
    }
}

QMap<QString, QString> Workflow::ExtractMetaDataFromElementComments(QDomElement* element)
{
    QMap<QString, QString> mapMetaData;
    QDomNodeList allSubNodes = element->childNodes();
    for (int subPos=0; subPos<allSubNodes.length(); subPos++) {
        QDomNode node = allSubNodes.at(subPos);
        if (!node.isComment()) continue;

        // comment detected - is it meta data?
        QDomComment comment = node.toComment();
        QString text = comment.data();
        if (text.contains("META-DATA")) {
            ParseMetaData(text, mapMetaData);
        }
    }

    return mapMetaData;
}

SCXMLState* Workflow::GetStateById(QString id)
{
    foreach(QObject* child, this->children()) {
        SCXMLState* state = dynamic_cast<SCXMLState*>(child);
        if (state->GetId() == id) return state;
    }
    return NULL;
}

void Workflow::CreateSceneObjects(QGraphicsScene* scene)
{
    foreach(QObject* child, this->children()) {
        SCXMLState* state = dynamic_cast<SCXMLState*>(child);
        QGraphicsItem* item = dynamic_cast<QGraphicsItem*>(state);
        scene->addItem(item);

        foreach(QAbstractTransition* abtran, state->transitions()) {
            SCXMLTransition* tran = dynamic_cast<SCXMLTransition*>(abtran);
            QGraphicsItem* itemTran = dynamic_cast<QGraphicsItem*>(tran);
            scene->addItem(itemTran);
        }
    }
}

void Workflow::ParseMetaData(QString text, QMap<QString, QString> &map)
{
    // may need to change this to handle more complex meta-data, for now looks for "[data]" and assumes a single = sign
    QRegExp rx(tr("\\[([^\\[]+)\\]"));
    int pos = 0;
    while ((pos = rx.indexIn(text, pos)) != -1) {
        QString metaData = rx.cap(1);
        QStringList parts = metaData.split('=');
        if (parts.length() > 1) {
            map.remove(parts[0]);
            map.insert(parts[0], parts[1]);
        }
        pos += rx.matchedLength();
    }
}
