#ifndef DATANODE_H
#define DATANODE_H

#include <string>
#include <unordered_map>
#include <boost/optional/optional.hpp>

#include "TreeNode.h"

class DataNode
{
public:

    enum class Type {
        Base,
        IfcObject,
        IfcClass
    };

    class Base : public TreeNode<Base>
    {
    public:
        Base(Type t = Type::Base): m_type(t) {}
        virtual ~Base(){}

        Type type() const {return m_type;}

        template<typename T>
        T* as(){ return (m_type == T::staticType)? static_cast<T*>(this) : nullptr; }

    protected:
        Type m_type;
    };

    class IfcClass : public Base
    {
    public:

        constexpr static Type staticType = Type::IfcClass;

        std::string m_ifcClass;
        int m_objectsCount;
        //std::vector<std::string> m_objectsGuids;

        IfcClass(const std::string& ifcClass, int nObjects)
            : Base(staticType), m_ifcClass(ifcClass), m_objectsCount(nObjects) {}
    };

    class IfcObject : public Base
    {
    public:

        constexpr static Type staticType = Type::IfcObject;

        std::string m_guid;
        std::string m_name;
        std::string m_ifcClass;

        IfcObject(const std::string& guid, const std::string& name, const std::string& ifcClass)
            : Base(staticType), m_guid(guid), m_name(name), m_ifcClass(ifcClass) {}
    };

    class Storey {
    public:

        std::string m_guid;
        std::string m_name;
        std::optional<double> m_elevation;
        std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> m_objectGuidsNamesByType; //object type _ list of (<guid, name>)

        Storey(){}
        Storey(const std::string& guid, const std::string& name, const std::optional<double>& elevation):m_guid(guid), m_name(name), m_elevation(elevation) {}

        bool operator < (const Storey& other) const
        {
            if (m_elevation.has_value() && other.m_elevation.has_value())
                return m_elevation.value() < other.m_elevation.value();
            return m_name < other.m_name;
        }
    };

};

#endif // DATANODE_H
