#ifndef EDGESUBLINE_H
#define EDGESUBLINE_H

#include "EdgeLocation.h"

namespace hoot
{

/**
 * This class represents a subline on an edge.
 *
 * This parallels WaySubline.
 */
class EdgeSubline
{
public:
  EdgeSubline(ConstEdgeLocationPtr start, ConstEdgeLocationPtr end);

  EdgeSubline(ConstNetworkEdgePtr e, double start, double end);

  shared_ptr<EdgeSubline> clone() const;

  static shared_ptr<EdgeSubline> createFullSubline(ConstNetworkEdgePtr e);

  const ConstNetworkEdgePtr& getEdge() const { return _start->getEdge(); }

  const ConstEdgeLocationPtr& getEnd() const { return _end; }

  /**
   * Returns the way location that is closer to the beginning of the way regardless of the
   * isBackwards() result.
   */
  ConstEdgeLocationPtr getFormer() const { return isBackwards() ? _end : _start; }

  /**
   * Returns the way location that is closer to the end of the way regardless of the
   * isBackwards() result.
   */
  ConstEdgeLocationPtr getLatter() const { return isBackwards() ? _start : _end; }

  const ConstEdgeLocationPtr& getStart() const { return _start; }

  bool isBackwards() const { return _end < _start; }

  bool isValid() const { return _start->isValid() && _end->isValid(); }

  bool isZeroLength() const { return _start == _end; }

  /**
   * swap start and end.
   */
  void reverse() { std::swap(_start, _end); }

  QString toString() const;

private:
  ConstEdgeLocationPtr _start, _end;
};

typedef shared_ptr<EdgeSubline> EdgeSublinePtr;
typedef shared_ptr<const EdgeSubline> ConstEdgeSublinePtr;

// not implemented
bool operator<(ConstEdgeSublinePtr, ConstEdgeSublinePtr);

}

#endif // EDGESUBLINE_H
