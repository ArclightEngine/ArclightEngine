#pragma once

namespace Arclight {

class Script {
public:
    Script(class Node& node);

    virtual void Start(){};
    virtual void Tick(){};

protected:
    class Node& m_node;
};

} // namespace Arclight
