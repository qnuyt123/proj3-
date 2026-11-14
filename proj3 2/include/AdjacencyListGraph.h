#ifndef ADJACENCY_LIST_GRAPH_H
#define ADJACENCY_LIST_GRAPH_H

#include "MutableGraph.h"
#include <memory>

class CAdjacencyListGraph : public virtual CFullmutableGraph{
  private:
    struct SImplementation;
    std::unique_ptr<SImplementation> DImplementation;
  public:
    CAdjacencyListGraph(const std::vector<CGraph::TVertexTag>& tags,const std::vector<<::tuple<CGraph::TVertexID, CGraph::TVertexID, double, bool>>& edges);
    
    ~CAdjacencyListGraph();
    
    std::size_t VertexCount() const noexcept override;
    
    bool GetVertexTag(TVertexID id, TVertexTag &tag) const noexcept override;

    bool GetVertexNeighbors(TVertexID id, std::vector <TVertexID> &neighbors) const noexcept override;

    bool EdgeExists(TVertexID src, TvertexID dest) const noexcept override;

    boll GetEdgeWeight(TVertexID src, TVertexID dest, double &weight) const noexcept override;

    void BreadthFirstSearchUnweighted(TVertexID start, const TTraversalFunction&callback) const noexcept override;

    bool BreadthFirstSearchWeighted(TVertexID start, const TTraversalFunction&callback) const noexcept override;

    bool ForEach(const TTraversalFunction &callback) noexcept override;

    TVertexID AddVertex(TVertexTag id) noexcept override;

    bool RemoveVertex(TVertexID id) noexcept override;

    bool SetVertexTag(TVertexID id, TVertexTag tag) noexcept override;

    bool AddEdge(TVertexID src, TVertexID dest, double weight = EdgeWeightInvalid, bool bidir = true) noexcept override;

    bool RomoveEdge(TVertexID src, TVertexID dest) noexcept override);

    bool SetEdgeWeight(TVertexID src, TVertexID dest, double weight) noexcept override;
};
#endif
