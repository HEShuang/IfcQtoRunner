#include "IfcStructureBuilder.h"

IfcStructureBuilder::IfcStructureBuilder() {}

std::unique_ptr<DataNode::Base> IfcStructureBuilder::buildTreeByStorey (
    IfcParse::IfcFile& ifcFile,
    const IfcSchemaStrategyBase& strategy
)
{
    auto spRootNode = std::make_unique<DataNode::Base>();
    auto ifcProjects = std::vector<IfcUtil::IfcBaseClass*>();
    strategy.getProjects(ifcFile, ifcProjects);

    HashRel hashRelContains, hashRelAggregates, hashRelVoids;

    strategy.extractRelationship_Contains(ifcFile, hashRelContains);
    strategy.extractRelationship_Aggregates(ifcFile, hashRelAggregates);
    strategy.extractRelationship_Voids(ifcFile, hashRelAggregates);


    //comparation class to compare unique pointer of Storey
    struct UPtrCompare
    {
        bool operator()(const std::unique_ptr<DataNode::Storey>& a,
                        const std::unique_ptr<DataNode::Storey>& b) const
        {
            if (a->m_elevation.has_value() && b->m_elevation.has_value())
                return a->m_elevation.value() < b->m_elevation.value();
            return a->m_name < b->m_name;
        }
    };

    std::map<DataNode::Base*, std::set<std::unique_ptr<DataNode::Storey>, UPtrCompare> > map_pBuilding_upStoreySet;
    std::map<std::string, DataNode::Storey*> map_guid_pStorey;

    //Build structure tree untill storey:
    //it's a storey -> store it for later use, return
    //it's not a storey -> create child node and add to parent, then process it's related objects recuirsively
    std::function<void(DataNode::Base*, IfcUtil::IfcBaseClass*)> buildTreeUntillStorey = [&](DataNode::Base* pParentNode, IfcUtil::IfcBaseClass* pIfcBase) {

        auto type = strategy.getTypeName(pIfcBase);
        auto guid = strategy.getGlobalId(pIfcBase);
        auto name = strategy.getName(pIfcBase);


        if (strategy.isStorey(pIfcBase))
        {
            auto upStorey = make_unique<DataNode::Storey>(guid, name, strategy.getStoreyElevation(pIfcBase));
            auto pStorey = upStorey.get();

            map_pBuilding_upStoreySet[pParentNode].insert(std::move(upStorey));
            map_guid_pStorey[guid] = pStorey;
            return;
        }

        //not a storey
        auto pChildNode = pParentNode->addChild(make_unique<DataNode::IfcObject>(guid, name, type));

        if(hashRelContains.contains(guid))
        {
            for (const auto& pRelatedObject : hashRelContains.at(guid))
                buildTreeUntillStorey(pChildNode, pRelatedObject);
        }

        if(hashRelAggregates.contains(guid))
        {
            for (const auto& pRelatedObject : hashRelAggregates.at(guid))
                buildTreeUntillStorey(pChildNode, pRelatedObject);
        }

    };

    for (auto pProject : ifcProjects)
        buildTreeUntillStorey(spRootNode.get(), pProject);


    //add the given object's guid to the given storey recursively
    std::function<void(DataNode::Storey*, IfcUtil::IfcBaseClass*)> addObjectToStorey = [&](DataNode::Storey* pStorey, IfcUtil::IfcBaseClass* pIfcBase) {

        auto objectGuid = strategy.getGlobalId(pIfcBase);
        auto type = strategy.getTypeName(pIfcBase);
        auto name = strategy.getName(pIfcBase);

        pStorey->m_objectGuidsNamesByType[type].emplace_back(objectGuid, name);


        //add related objects to storey recursively
        if(hashRelContains.contains(objectGuid))
            for (const auto& pRelatedObject : hashRelContains.at(objectGuid))
                addObjectToStorey(pStorey, pRelatedObject);

        if(hashRelAggregates.contains(objectGuid))
            for (const auto& pRelatedObject : hashRelAggregates.at(objectGuid))
                addObjectToStorey(pStorey, pRelatedObject);

        if(hashRelVoids.contains(objectGuid))
            for (const auto& pRelatedObject : hashRelVoids.at(objectGuid))
                addObjectToStorey(pStorey, pRelatedObject);
    };

    //for each storey, add its related objects recursively
    for (const auto& pair : map_guid_pStorey)
    {
        const auto& storeyGuid = pair.first;
        auto pStorey = pair.second;

        if(hashRelContains.contains(storeyGuid))
            for (const auto& pRelatedObject : hashRelContains.at(storeyGuid))
                addObjectToStorey(pStorey, pRelatedObject);

        if(hashRelAggregates.contains(storeyGuid))
            for (const auto& pRelatedObject : hashRelAggregates.at(storeyGuid))
                addObjectToStorey(pStorey, pRelatedObject);
    }

    //complete structural tree: storey nodes and ifcClass nodes
    for (const auto& pair : map_pBuilding_upStoreySet)
    {
        auto pBuildingNode = pair.first;
        for (const auto& upStorey : pair.second)
        {
            //create storey node and add to building node
            auto pStoreyNode = pBuildingNode->addChild( make_unique<DataNode::IfcObject>(upStorey->m_guid, upStorey->m_name, "IfcBuildingStorey") );

            //create ifcClass nodes and add to storey node
            for (auto pair2 : upStorey->m_objectGuidsNamesByType)
            {
                auto pClassNode = pStoreyNode->addChild( make_unique<DataNode::IfcClass>(pair2.first, pair2.second.size()) );

                //create ifcObject nodes of the same ifc class
                for (const auto& pair_guid_name : pair2.second)
                    pClassNode->addChild( make_unique<DataNode::IfcObject>(pair_guid_name.first, pair_guid_name.second, pair2.first));

            }
        }
    }

    return spRootNode;
}
