#ifndef IFCSCHEMA_STRATEGY_BASE_H
#define IFCSCHEMA_STRATEGY_BASE_H

#include <vector>
#include <unordered_map>

namespace IfcParse { class IfcFile; }
namespace IfcUtil { class IfcBaseClass; }

// HashRel store ifc relationship which maps the GUID of relating object to a vector of related instance IDs
using HashRel = std::unordered_map<std::string, std::vector<IfcUtil::IfcBaseClass*>>;

class IfcSchemaStrategyBase {
public:
    virtual ~IfcSchemaStrategyBase() = default;

    // Relationship extraction now populates the ID-based HashRel
    virtual void extractRelationship_Contains(IfcParse::IfcFile& file, HashRel& map) const = 0;
    virtual void extractRelationship_Aggregates(IfcParse::IfcFile& file, HashRel& map) const = 0;
    virtual void extractRelationship_Voids(IfcParse::IfcFile& file, HashRel& map) const = 0;

    // --- Instance Getters and Type Checks ---
    virtual void getProjects(IfcParse::IfcFile& file, std::vector<IfcUtil::IfcBaseClass*>& ifcProjects) const = 0;
    virtual bool isStorey(IfcUtil::IfcBaseClass* obj) const = 0;

    // --- Property Accessors ---
    virtual std::string getGlobalId(IfcUtil::IfcBaseClass* obj) const = 0;
    virtual std::string getName(IfcUtil::IfcBaseClass* obj) const = 0;
    virtual std::string getTypeName(IfcUtil::IfcBaseClass* obj) const = 0;
    virtual std::optional<double> getStoreyElevation(IfcUtil::IfcBaseClass* obj) const = 0;
};

#endif // IFCSCHEMA_STRATEGY_BASE_H
