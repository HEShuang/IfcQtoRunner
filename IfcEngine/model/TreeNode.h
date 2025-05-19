#ifndef TREENODE_H
#define TREENODE_H

#include <memory.h>
#include <vector>

/*
 * * CRTP pattern template
 */
template <class Derived>
class TreeNode
{
public:
    TreeNode() = default;
    ~TreeNode() = default;

    TreeNode(const TreeNode&) = delete;
    TreeNode& operator=(const TreeNode&) = delete;

    TreeNode(TreeNode&&) noexcept = default;
    TreeNode& operator=(TreeNode&&) noexcept = default;

    Derived* addChild(std::unique_ptr<Derived>&& child)
    {
        Derived* raw = child.get();
        child->m_parent = static_cast<Derived*>(this);
        m_children.push_back(std::move(child));
        return raw;
    }

    const auto& getChildren()
    {
        return m_children;
    }

protected:
    Derived* m_parent = nullptr;
    std::vector<std::unique_ptr<Derived>> m_children;
};

#endif // TREENODE_H
