#include "node.h"

// implementation of base Node class
Node::Node()
    :children(), geometry(nullptr)
{}


Node::Node(const Node &n)
    :children(), geometry(n.geometry)
{
    // call copy constructor of each child node of n and add it to curr node's children
    for (const uPtr<Node> &child : n.children) {
        TranslateNode* t = dynamic_cast<TranslateNode*>(child.get());
        RotateNode* r = dynamic_cast<RotateNode*>(child.get());
        ScaleNode* s = dynamic_cast<ScaleNode*>(child.get());
        if (t != nullptr) {
            this->children.push_back(mkU<TranslateNode>(*t));
        } else if (r != nullptr) {
            this->children.push_back((mkU<RotateNode>(*r)));
        } else if (s != nullptr) {
            this->children.push_back((mkU<ScaleNode>(*s)));
        }
    }
}


Node& Node::operator=(const Node &n)
{
    this->children.clear();
    this->geometry = n.geometry;
    for (const uPtr<Node> &child : n.children) {
        TranslateNode* t = dynamic_cast<TranslateNode*>(child.get());
        RotateNode* r = dynamic_cast<RotateNode*>(child.get());
        ScaleNode* s = dynamic_cast<ScaleNode*>(child.get());
        if (t != nullptr) {
            this->children.push_back(mkU<TranslateNode>(*t));
        } else if (r != nullptr) {
            this->children.push_back((mkU<RotateNode>(*r)));
        } else if (s != nullptr) {
            this->children.push_back((mkU<ScaleNode>(*s)));
        }
    }
    return *this;
}

Node& Node::addChild(uPtr<Node> child) {
    Node &ref = *child;
    children.push_back(std::move(child));
    return ref;
}


Node::~Node()
{}


// implementation of TranslateNode
TranslateNode::TranslateNode()
    :TranslateNode(0.f, 0.f, 0.f)
{}


TranslateNode::TranslateNode(float x, float y, float z)
    :x_translation(x), y_translation(y), z_translation(z)
{}


TranslateNode::TranslateNode(const TranslateNode &n)
    :Node(n), x_translation(n.x_translation), y_translation(n.y_translation), z_translation(n.z_translation)
{}


TranslateNode& TranslateNode::operator=(const TranslateNode &n) {
    Node::operator=(n);
    this->x_translation = n.x_translation;
    this->y_translation = n.y_translation;
    this->z_translation = n.z_translation;
    return *this;
}


glm::mat4 TranslateNode::getTransformation() const
{
    return glm::translate(glm::mat4(1.0f), glm::vec3(x_translation, y_translation, z_translation));
}


TranslateNode::~TranslateNode()
{}


// implementation of RotateNode
RotateNode::RotateNode()
    :RotateNode(0.f, 0.f)
{}


RotateNode::RotateNode(float theta, float phi)
    :theta(theta), phi(phi)
{}


RotateNode::RotateNode(const RotateNode &n)
    :Node(n), theta(n.theta), phi(n.phi)
{}


RotateNode& RotateNode::operator=(const RotateNode &n) {
    Node::operator=(n);
    this->theta = n.theta;
    this->phi = n.phi;
    return *this;
}


glm::mat4 RotateNode::getTransformation() const
{
    glm::mat4 rotatePhi = glm::rotate(glm::mat4(1.f), glm::radians(phi), glm::vec3(1.f, 0.f, 0.f));
    glm::mat4 rotateTheta = glm::rotate(glm::mat4(1.f), glm::radians(theta), glm::vec3(0.f, 1.f, 0.f));
    return rotateTheta * rotatePhi;
}


RotateNode::~RotateNode()
{}


// implementation of ScaleNode
ScaleNode::ScaleNode()
    :ScaleNode(0.f, 0.f, 0.f)
{}


ScaleNode::ScaleNode(float x, float y, float z)
    :x_scale(x), y_scale(y), z_scale(z)
{}


ScaleNode::ScaleNode(const ScaleNode &n)
    :Node(n), x_scale(n.x_scale), y_scale(n.y_scale), z_scale(n.z_scale)
{}


ScaleNode& ScaleNode::operator=(const ScaleNode &n) {
    Node::operator=(n);
    this->x_scale = n.x_scale;
    this->y_scale = n.y_scale;
    this->z_scale = n.z_scale;
    return *this;
}


glm::mat4 ScaleNode::getTransformation() const
{
    return glm::scale(glm::mat4(1.0f), glm::vec3(x_scale, y_scale, z_scale));
}


ScaleNode::~ScaleNode()
{}
