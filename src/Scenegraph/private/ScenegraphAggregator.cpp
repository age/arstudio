#include <ScenegraphAggregator.hpp>

namespace arstudio {
ScenegraphAggregator::ScenegraphAggregator (QObject * parent)
  : QObject (parent),
  m_min_frame (1),
  m_max_frame (1)
{
}

int
ScenegraphAggregator::min_frame (void)
{
  return m_min_frame;
}

int
ScenegraphAggregator::max_frame (void)
{
  return m_max_frame;
}

void
ScenegraphAggregator::rebuild_frames ()
{
  m_frames.clear ();
  for (Scenegraph * s : m_scenegraph_list)
    m_frames += s->frames ();
  recalculate_limits ();
}

void
ScenegraphAggregator::recalculate_limits ()
{
  if (m_frames.empty ())
    return;
  QList<int> v = m_frames.values ();
  qSort (v);

  m_max_frame = v.last ();
  max_frame_changed ();
  m_min_frame = v.first ();
  min_frame_changed ();
}

void
ScenegraphAggregator::add_scenegraph (Scenegraph * scenegraph)
{
  m_scenegraph_list.append (scenegraph);
  connect (scenegraph, &Scenegraph::sequences_changed,
           this, &ScenegraphAggregator::rebuild_frames);
}

void
ScenegraphAggregator::remove_scenegraph (Scenegraph * scenegraph)
{
  m_scenegraph_list.removeOne (scenegraph);
  rebuild_frames ();
}

bool
ScenegraphAggregator::valid_frame (int frame)
{
  return m_frames.contains (frame);
}

void
ScenegraphAggregator::signal_frame (int frame)
{
  change_frame (frame);
}
}
