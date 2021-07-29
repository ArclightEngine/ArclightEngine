#pragma once

#include <list>
#include <memory>
#include <string>

#include <Arclight/Core/Job.h>
#include <Arclight/Core/Timer.h>

namespace Arclight {

class Node 
    : public Job {
public:
    Node();
    virtual ~Node() = default;

    template<typename N, typename... Args>
    static inline std::shared_ptr<Node> Create(Args... args){
        return std::shared_ptr<Node>(new N(args...));
    }

    void Run() final override;

    virtual void Start();
    virtual void Tick();

    inline const std::string& Name() const { return m_name; }

protected:
    inline float Elapsed() const { return m_elapsedTime; }

    inline Node* Root() { return m_root; }
    inline Node* Parent() { return m_parent; }

    inline void AddChild(std::shared_ptr<Node> child) {
        child->m_root = m_root;
        child->m_parent = this;
        m_children.push_back(std::move(child));
    }

    Node* m_root = nullptr;
    Node* m_parent = nullptr;

    std::list<std::shared_ptr<Node>> m_children;
    
private:
    float m_elapsedTime;
    Timer m_timer;

    std::string m_name;
};

}