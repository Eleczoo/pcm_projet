//
//  path.hpp
//
//  Copyright (c) 2022 Marcelo Pasin. All rights reserved.
//

#include "graph.hpp"
#include <iostream>

#ifndef _path_hpp
#define _path_hpp

class Path {
private:
  int _size;
  int _distance;
  int *_nodes;
  int _nodes_bitfield;
  Graph *_graph;

public:
  ~Path() {
    clear();
    //delete[] _nodes;
    _nodes = 0;
    _graph = 0;
  }

  Path(Graph *graph) {
    _graph = graph;
    // TODO: Fixed size nodes (number of cities)
    _nodes = new int[max() + 1];
    _nodes_bitfield = 0;
    _distance = 0;
    clear();
  }

  // default constructor
  Path() : _size(0), _distance(0), _nodes(0), _nodes_bitfield(0), _graph(0) {}

  int max() const { return _graph->size(); }
  int size() const { return _size; }
  bool leaf() const { return (_size == max()); }
  int distance() const { return _distance; }
  void clear() { _size = _distance = 0; }

  void add(int node) {
    // std::cout << "path.add() size before : " << _size << std::endl;
    if (_size <= max()) {
      if (_size) {
        int last = _nodes[_size - 1];
        int distance = _graph->distance(last, node);
        _distance += distance;
      }
      _nodes[_size++] = node;
      _nodes_bitfield |= 1 << node;
    }
    // std::cout << "path.add() _size after : " << _size << std::endl;
  }

  void pop() {
    if (_size) {
      int last = _nodes[--_size];
      if (_size) {
        int node = _nodes[_size - 1];
        int distance = _graph->distance(node, last);
        _distance -= distance;
        _nodes_bitfield &= ~(1 << last);
      }
    }
  }

  bool contains(int node) const { return (_nodes_bitfield & (1 << node)); }

  void copy(Path *o) {
    if (max() != o->max()) {
      //delete[] _nodes;
      _nodes = new int[o->max() + 1];
    }
    _graph = o->_graph;
    // std::cout << "path.copy() copied size from graph" << o->_size <<
    // std::endl;
    _size = o->_size;
    // std::cout << "path.copy() actually " << _size << std::endl;
    _distance = o->_distance;
    _nodes_bitfield = o->_nodes_bitfield;
    for (int i = 0; i < _size; i++)
      _nodes[i] = o->_nodes[i];
  }

  void print(std::ostream &os) const {
    os << '[' << _distance;
    for (int i = 0; i < _size; i++)
      os << (i ? ',' : ':') << ' ' << _nodes[i];
    os << ']';
  }
};

std::ostream &operator<<(std::ostream &os, Path *p) {
  p->print(os);
  return os;
}

#endif // _path_hpp