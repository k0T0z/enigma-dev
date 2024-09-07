/********************************************************************************\
** **
**  Copyright (C) 2024 Saif Kandil (k0T0z) **
** **
**  This file is a part of the ENIGMA Development Environment. **
** **
** **
**  ENIGMA is free software: you can redistribute it and/or modify it under the
**
**  terms of the GNU General Public License as published by the Free Software **
**  Foundation, version 3 of the license or any later version. **
** **
**  This application and its source code is distributed AS-IS, WITHOUT ANY **
**  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS **
**  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more **
**  details. **
** **
**  You should have recieved a copy of the GNU General Public License along **
**  with this code. If not, see <http://www.gnu.org/licenses/> **
** **
**  ENIGMA is an environment designed to create games and other programs with a
**
**  high-level, fully compilable language. Developers of ENIGMA or anything **
**  associated with ENIGMA are in no way responsible for its users or **
**  applications created by its users, or damages caused by the environment **
**  or programs made in the environment. **
** **
\********************************************************************************/

#ifndef ENIGMA_VISUAL_SHADER_H
#define ENIGMA_VISUAL_SHADER_H

#include <map>
#include <memory>  // For std::shared_ptr
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <cmath>
#include <limits>

#define T_E 2.71828182845904523536 // Euler's number
#define T_PHI 1.618033988749895 // Golden ratio

bool t_are_almost_equal(const float& a, const float& b);

struct TVector2 {
  float x;
  float y;

  TVector2() : x(0.0f), y(0.0f) {}

  TVector2(const float& x, const float& y) : x(x), y(y) {}

  bool are_almost_equal(const TVector2& vector) const { return t_are_almost_equal(x, vector.x) && t_are_almost_equal(y, vector.y); }

  bool operator==(const TVector2& v) const { return x == v.x && y == v.y; }
};

struct TVector3 {
  float x;
  float y;
  float z;

  TVector3() : x(0.0f), y(0.0f), z(0.0f) {}

  TVector3(const float& x, const float& y, const float& z) : x(x), y(y), z(z) {}

  bool are_almost_equal(const TVector3& vector) const {
    return t_are_almost_equal(x, vector.x) && t_are_almost_equal(y, vector.y) && t_are_almost_equal(z, vector.z);
  }

  bool operator==(const TVector3& v) const { return x == v.x && y == v.y && z == v.z; }
};

struct TVector4 {
  float x;
  float y;
  float z;
  float w;

  TVector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

  TVector4(const float& x, const float& y, const float& z, const float& w) : x(x), y(y), z(z), w(w) {}

  bool are_almost_equal(const TVector4& vector) const {
    return t_are_almost_equal(x, vector.x) && t_are_almost_equal(y, vector.y) && t_are_almost_equal(z, vector.z) && t_are_almost_equal(w, vector.w);
  }

  bool operator==(const TVector4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
};

struct TColor {
  float r;
  float g;
  float b;
  float a;

  TColor() : r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}

  TColor(const float& r, const float& g, const float& b, const float& a) : r(r), g(g), b(b), a(a) {}

  bool are_almost_equal(const TColor& color) const {
    return t_are_almost_equal(r, color.r) && t_are_almost_equal(g, color.g) && t_are_almost_equal(b, color.b) && t_are_almost_equal(a, color.a);
  }

  bool operator==(const TColor& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
};

using TVariant = std::variant<std::monostate, float, int, TVector2, TVector3, TVector4, bool, std::string>;

class VisualShaderNode;

class VisualShader {
 public:
  struct Connection {
    int from_node = 0;
    int from_port = 0;
    int to_node = 0;
    int to_port = 0;
  };

  enum {
    NODE_ID_INVALID = -1,
    NODE_ID_OUTPUT = 0,
  };

  union ConnectionKey {
// Ignore the warning about the anonymous struct.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    struct {
      uint64_t node : 32;
      uint64_t port : 32;
    };
#pragma GCC diagnostic pop
    uint64_t key = 0;
    bool operator<(const ConnectionKey& key) const { return this->key < key.key; }
  };

  VisualShader();

  /**
   * @brief Each node in the graph has its id and this
   *        function generates it.
   *
   * @param type The type of shader. This will be used to
   *             access the correct graph in the array. See
   *             @c VisualShader::graph down below. The type
   *             will be @c VisualShader::Type::TYPE_FRAGMENT
   *             for now.
   *
   * @return int The id of the new node.
   */
  int get_valid_node_id() const;

  /**
   * @brief The function adds a node to the graph.
   *
   * @param type The type of shader. This will be used to
   *             access the correct graph in the array. See
   *             @c VisualShader::graph down below. The type
   *             will be @c VisualShader::Type::TYPE_FRAGMENT
   *             for now.
   *
   * @param node The node to add.
   * @param position The position of the node in the graph editor.
   *                 This is an array of two integers. @todo Should
   *                 this be a @c Vector2 type?
   *
   * @param id The id of the node. This will be generated by
   *           @c VisualShader::get_valid_node_id() function.
   */
  void add_node(const std::shared_ptr<VisualShaderNode>& node, const TVector2& position, const int& id);
  int find_node_id(const std::shared_ptr<VisualShaderNode>& node) const;
  void remove_node(const int& id);

  /**
   * @brief Get the node object
   *
   * @param type The type of shader. This will be used to
   *             access the correct graph in the array. See
   *             @c VisualShader::graph down below. The type
   *             will be @c VisualShader::Type::TYPE_FRAGMENT
   *             for now.
   *
   * @param id The id of the node.
   * @return std::shared_ptr<VisualShaderNode> The node object.
   */
  std::shared_ptr<VisualShaderNode> get_node(const int& id) const;

  /**
   * @brief Check if two ports are compatible.
   *
   * @note Ports: PORT_TYPE_SCALAR, PORT_TYPE_SCALAR_INT, PORT_TYPE_SCALAR_UINT,
   *              PORT_TYPE_VECTOR_2D, PORT_TYPE_VECTOR_3D, PORT_TYPE_VECTOR_4D,
   * and PORT_TYPE_BOOLEAN are compatible with each other. Other types are also
   * compatible with each other but not with the previous types.
   */
  bool is_port_types_compatible(const int& p1, const int& p2) const;

  bool is_nodes_connected_relatively(const int& node, const int& target) const;

  bool can_connect_nodes(const int& from_node, const int& from_port, const int& to_node, const int& to_port) const;
  bool connect_nodes(const int& from_node, const int& from_port, const int& to_node, const int& to_port);
  void disconnect_nodes(const int& from_node, const int& from_port, const int& to_node, const int& to_port);

  bool generate_shader() const;

  bool generate_shader_for_each_node(std::string& global_code, std::string& global_code_per_node,
                                     std::string& func_code,
                                     const std::map<ConnectionKey, const Connection*>& input_connections,
                                     const std::map<ConnectionKey, const Connection*>& output_connections,
                                     const int& node_id, std::unordered_set<int>& processed,
                                     std::unordered_set<std::string>& global_processed) const;

  std::string get_code() const;

 private:
  struct Node {
    std::shared_ptr<VisualShaderNode> node;
    TVector2 position;
    std::vector<int> prev_connected_nodes;
    std::vector<int> next_connected_nodes;
  };

  /**
   * @brief An array of graphs, each graph is a different
   *        type of shader (vertex, fragment, etc).
   *
   */
  struct Graph {
    std::map<int, VisualShader::Node> nodes;
    std::vector<VisualShader::Connection> connections;
  } graph;

  // The `code` variable will hold the generated shader code.
  mutable std::string code;

  void set_code(const std::string& code) const;
};

class VisualShaderNode {
 public:
  enum PortType {
    PORT_TYPE_SCALAR,
    PORT_TYPE_SCALAR_INT,
    PORT_TYPE_SCALAR_UINT,
    PORT_TYPE_VECTOR_2D,
    PORT_TYPE_VECTOR_3D,
    PORT_TYPE_VECTOR_4D,
    PORT_TYPE_BOOLEAN,
    PORT_TYPE_ENUM_SIZE,
  };

  enum Category {
    CATEGORY_NONE,
    CATEGORY_OUTPUT,
    CATEGORY_COLOR,
    CATEGORY_CONDITIONAL,
    CATEGORY_INPUT,
    CATEGORY_SCALAR,
    CATEGORY_UTILITY,
    CATEGORY_VECTOR,
    CATEGORY_ENUM_SIZE
  };

  VisualShaderNode();
  virtual ~VisualShaderNode() = default;

  bool is_simple_decl() const;

  virtual std::string get_caption() const = 0;

  virtual std::string generate_global([[maybe_unused]] const int& id) const;
  virtual std::string generate_global_per_node([[maybe_unused]] const int& id) const;
  virtual std::string generate_global_per_func([[maybe_unused]] const int& id) const;

  // If no output is connected, the output var passed will be empty. If no input
  // is connected and input is NIL, the input var passed will be empty.
  virtual std::string generate_code([[maybe_unused]] const int& id,
                                    [[maybe_unused]] const std::vector<std::string>& input_vars,
                                    [[maybe_unused]] const std::vector<std::string>& output_vars) const = 0;

  virtual int get_input_port_count() const = 0;
  virtual VisualShaderNode::PortType get_input_port_type(const int& port) const = 0;
  virtual std::string get_input_port_name(const int& port) const = 0;
  virtual int get_default_input_port([[maybe_unused]] const VisualShaderNode::PortType& type) const;
  virtual bool is_input_port_default([[maybe_unused]] const int& port) const;

  virtual void set_input_port_default_value(const int& port, const TVariant& value,
                                            const TVariant& prev_value = TVariant());
  TVariant get_input_port_default_value(const int& port) const;

  virtual int get_output_port_count() const = 0;
  virtual VisualShaderNode::PortType get_output_port_type(const int& port) const = 0;
  virtual std::string get_output_port_name(const int& port) const = 0;

  virtual bool is_show_prop_names() const;

  virtual bool is_output_port_expandable(const int& port) const;
  void set_output_ports_expanded(const std::vector<int>& data);
  std::vector<int> get_output_ports_expanded() const;
  void set_output_port_expanded(const int& port, const bool& expanded);
  bool is_output_port_expanded(const int& port) const;
  int get_expanded_output_port_count() const;

  bool is_output_port_connected(const int& port) const;
  void set_output_port_connected(const int& port, const bool& connected);
  bool is_input_port_connected(const int& port) const;
  void set_input_port_connected(const int& port, const bool& connected);

  virtual bool has_output_port_preview([[maybe_unused]] const int& port) const;

  virtual VisualShaderNode::Category get_category() const;

  virtual std::vector<std::string> get_editable_properties() const;
  virtual std::unordered_map<std::string, std::string> get_editable_properties_names() const;

  virtual std::string get_warning() const;

 protected:
  bool simple_decl;

 private:
  std::unordered_map<int, TVariant> default_input_values;

  std::unordered_map<int, bool> connected_input_ports;
  std::unordered_map<int, int> connected_output_ports;
  std::unordered_map<int, bool> expanded_output_ports;
};

/*************************************/
/* Input Node                        */
/*************************************/

class VisualShaderNodeInput : public VisualShaderNode {
 public:
  VisualShaderNodeInput();
  virtual ~VisualShaderNodeInput() = default;

  virtual std::string generate_code([[maybe_unused]] const int& id,
                                    [[maybe_unused]] const std::vector<std::string>& input_vars,
                                    [[maybe_unused]] const std::vector<std::string>& output_vars) const override;

  virtual int get_input_port_count() const override;
  virtual VisualShaderNode::PortType get_input_port_type([[maybe_unused]] const int& port) const override;
  virtual std::string get_input_port_name([[maybe_unused]] const int& port) const override;

  virtual int get_output_port_count() const override;
  virtual VisualShaderNode::PortType get_output_port_type(const int& port) const override;
  virtual std::string get_output_port_name([[maybe_unused]] const int& port) const override;

  void set_input_name(const std::string& name);
  std::string get_input_name() const;
  std::string get_input_real_name() const;

  virtual std::string get_caption() const override;

  VisualShaderNode::PortType get_input_type_by_name(const std::string& name) const;

 private:
  struct Port {
    PortType type = VisualShaderNode::PortType::PORT_TYPE_ENUM_SIZE;
    std::string name;
    std::string string_value;
  };

  static const VisualShaderNodeInput::Port ports[];
  static const VisualShaderNodeInput::Port preview_ports[];

  std::string input_name;
};

/*************************************/
/* Output Node                       */
/*************************************/

class VisualShaderNodeOutput : public VisualShaderNode {
 public:
  VisualShaderNodeOutput();
  virtual ~VisualShaderNodeOutput() = default;

  virtual std::string generate_code([[maybe_unused]] const int& id,
                                    [[maybe_unused]] const std::vector<std::string>& input_vars,
                                    [[maybe_unused]] const std::vector<std::string>& output_vars) const override;

  virtual int get_input_port_count() const override;
  virtual VisualShaderNode::PortType get_input_port_type(const int& port) const override;
  virtual std::string get_input_port_name(const int& port) const override;

  virtual int get_output_port_count() const override;
  virtual VisualShaderNode::PortType get_output_port_type([[maybe_unused]] const int& port) const override;
  virtual std::string get_output_port_name([[maybe_unused]] const int& port) const override;

  virtual std::string get_caption() const override;

 private:
  struct Port {
    VisualShaderNode::PortType type = VisualShaderNode::PortType::PORT_TYPE_ENUM_SIZE;
    std::string name;
    std::string string_value;
  };

  static const VisualShaderNodeOutput::Port ports[];
};

#endif  // ENIGMA_VISUAL_SHADER_H
