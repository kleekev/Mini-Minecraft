#pragma once
#include <QTreeWidgetItem>
#include <memory>
#include <vector>
#include "smartpointerhelp.h"
#include "cube.h"

class Node {
protected:
    Node();
public:
    Node(const Node &n); // copy constructor
    Node& operator=(const Node &n);

    std::vector<uPtr<Node>> children; // vector of child nodes in the scene graph

    Cube* geometry; // references the geometry to draw in this node, could be nullptr if we are not drawing

    // abstract function that must be implemented for instantiation of object
    virtual glm::mat4 getTransformation() const = 0;

    // adds the passed in unique ptr into the node's children vector via std::moves
    // returns reference to the child added into the children array
    Node& addChild(uPtr<Node> child);

    virtual ~Node();
};


class TranslateNode : public Node {
public:
    float x_translation;
    float y_translation;
    float z_translation;
    TranslateNode();
    TranslateNode(float x, float y, float z);
    TranslateNode(const TranslateNode &n); // copy constructor
    TranslateNode& operator=(const TranslateNode &n);

    glm::mat4 getTransformation() const override;
    virtual ~TranslateNode();
};


class RotateNode : public Node {
public:
    float theta; // degrees of rotation in the horizontal plane
    float phi; // degrees of rotation in the vertical plane

    RotateNode();
    RotateNode(float theta, float phi);
    RotateNode(const RotateNode &n); // copy constructor
    RotateNode& operator=(const RotateNode &n);

    glm::mat4 getTransformation() const override;
    virtual ~RotateNode();
};


class ScaleNode : public Node {
public:
    float x_scale;
    float y_scale;
    float z_scale;
    ScaleNode();
    ScaleNode(float x, float y, float z);
    ScaleNode(const ScaleNode &n); // copy constructor
    ScaleNode& operator=(const ScaleNode &n);

    glm::mat4 getTransformation() const override;
    virtual ~ScaleNode();
};
