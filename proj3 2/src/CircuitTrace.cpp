#include "CircuitTrace.h"
#include <algorithm>
#include <utility>
#include <numeric>

struct CCircuitTrace::SImplementation{
    TCircuitName DName;
    std::vector<SCircuitLocation> DLocations;
    std::vector<SCircuitTraceEdge> DEdges;

    SImplementation() = default;

    SImplementation(const SImplementation &other)
        : DName(other.DName),
          DLocations(other.DLocations),
          DEdges(other.DEdges) {}

    SImplementation(SImplementation &&other) noexcept
        : DName(std::move(other.DName)),
          DLocations(std::move(other.DLocations)),
          DEdges(std::move(other.DEdges)) {}
};
CCircuitTrace::CCircuitTrace()
    : DImplementation(std::make_unique<SImplementation>()) {}

CCircuitTrace::CCircuitTrace(const CCircuitTrace &trace)
    : DImplementation(std::make_unique<SImplementation>(*trace.DImplementation)) {}

CCircuitTrace::CCircuitTrace(CCircuitTrace &&trace) noexcept
    : DImplementation(std::move(trace.DImplementation)) {}
TCircuitName CCircuitTrace::Name() const {
    return DImplementation->DName;
}
TCircuitName CCircuitTrace::Name(const TCircuitName &name) {
    if (!name.empty()) {
        DImplementation->DName = name;
    }
    return DImplementation->DName;
}
std::size_t CCircuitTrace::LocationCount() const {
    return DImplementation->DLocations.size();
}
bool CCircuitTrace::Location(
    std::size_t index, SCircuitLocation &location) const 
{
    if (index >= DImplementation->DLocations.size()) {
        return false;
        }
        location = DImplementation->DLocations[index];
        return true;
}
std::size_t CCircuitTrace::EdgeCount() const {
    return DImplementation->DEdges.size();
}
bool CCircuitTrace::Edge(
    std::size_t index, SCircuitTraceEdge &edge) const {
    if (index >= DImplementation->DEdges.size()) {
        return false;
    }
    edge = DImplementation->DEdges[index];
    return true;
}
bool CCircuitTrace::SourceDestinationAsEdge(
    SCircuitTraceEdge &edge) const {
    if (DImplementation->DEdges.empty()) {
        return false;
    }
    edge.DSource = DImplementation->DEdges.front().DSource;
    edge.DDestination = DImplementation->DEdges.back().DDestination;
    return true;
}
bool CCircuitTrace::AddEdge(const SCircuitTraceEdge &edge) {
   for (const auto &existing : DImplementation->DEdges) 
{
        if ((existing.DSource == edge.DSource && existing.DDestination == edge.DDestination) ||
            (existing.DSource == edge.DDestination && existing.DDestination == edge.DSource)) {
            return false; // Edge already exists
        }
    }
    DImplementation->DEdges.push_back(edge);
    
    if (std::find(DImplementation->DLocations.begin(),
                  DImplementation->DLocations.end(), 
                  edge.DSource) == DImplementation->DLocations.end()) {
        DImplementation->DLocations.push_back(edge.DSource);
                    
    }
    if (std::find(DImplementation->DLocations.begin(),
                  DImplementation->DLocations.end(), 
                  edge.DDestination) == DImplementation->DLocations.end()) {
        DImplementation->DLocations.push_back(edge.DDestination);
    }
    return true;
}
bool CCircuitTrace::IsBroken() const {
    const auto &edges = DImplementation->DEdges;
    for(std::size_t i = 1; i < edges.size(); ++i) {
        if (edges[i-1].DDestination != edges[i].DSource) {
            return true; // Found a break in the trace
        }
    }
    return false; // No breaks found 
}
bool CCircuitTrace::Fix() {
    const auto originalEdges = DImplementation->DEdges;
    if (originalEdges.empty()) {
        return false; // Nothing to fix
    }

    CCircuitTrace fixed;
    fixed.Name(DImplementation->DName);

    for (std::size_t i = 0; i < originalEdges.size(); ++i) {
        fixed.AddEdge(originalEdges[i]);
        if (i + 1 == originalEdges.size()) {
            continue; // No next edge to check
        }

        const auto start = originalEdges[i].DDestination;
        const auto end = originalEdges[i + 1].DSource;
        if (start == end) {
            continue; // No break between edges
        }
        const auto dx = start.DX > end.DX 
            ? start.DX - end.DX : end.DX - start.DX;
        const auto dy = start.DY > end.DY 
            ? start.DY - end.DY : end.DY - start.DY;
        const auto diagonal = std::min(dx, dy);

        SCircuitLocation middle = start;
        if (end.DX > start.DX) middle.DX += diagonal;
        else middle.DX -= diagonal;
        if (end.DY > start.DY) middle.DY += diagonal;
        else middle.DY -= diagonal;
        if (middle != start) {
            fixed.AddEdge({start, middle});
        }
        if (middle != end) {
            fixed.AddEdge({middle, end}); 
        }
    }

DImplementation = std::move(fixed.DImplementation);
return true;
}
CCircuitTrace CCircuitTrace::Simplify() const {
    CCircuitTrace result;
    result.Name(DImplementation->DName);
    const auto &edges = DImplementation->DEdges;
    if (edges.empty()) {
        return result; // Return empty trace if no edges
    }
   auto sameDirection = [](const SCircuitLocation &a, const SCircuitLocation &b, const SCircuitLocation &c) {
        const long double dx1 = static_cast<long double>(b.DX) - a.DX;const long double dy1 = static_cast<long double>(b.DY) - a.DY;
        const long double dx2 = static_cast<long double>(c.DX) - b.DX;const long double dy2 = static_cast<long double>(c.DY) - b.DY;
        return dx1 * dy2 == dy1 * dx2 && dx1 * dx2 + dy1 * dy2 > 0; // Check if the direction is the same
    };
    SCircuitTraceEdge pending = edges.front();
    for (std::size_t i = 1; i < edges.size(); ++i) {
        if (pending.DDestination == edges[i].DSource && sameDirection(pending.DSource, pending.DDestination, edges[i].DDestination)) {
            pending.DDestination = edges[i].DDestination; // Extend the current edge
        } else {
            result.AddEdge(pending); // Add the current edge to the result
            pending = edges[i]; // Start a new edge
        }
    }
    result.AddEdge(pending); // Add the last edge
    return result;
}
CCircuitTrace CCircuitTrace::Elaborate() const {
    CCircuitTrace result;
    result.Name(DImplementation->DName);
    for (const auto &edge : DImplementation->DEdges) {
        const auto dx = edge.DSource.DX > edge.DDestination.DX ? edge.DSource.DX - edge.DDestination.DX : edge.DDestination.DX - edge.DSource.DX;
        const auto dy = edge.DSource.DY > edge.DDestination.DY ? edge.DSource.DY - edge.DDestination.DY : edge.DDestination.DY - edge.DSource.DY;
        const auto steps = std::max(dx, dy);
        
        if (steps == 0) {
            result.AddEdge(edge); // No steps needed, add the edge directly
            continue;
        }
        const auto stepX = dx / steps;
        const auto stepY = dy / steps;
        auto previous = edge.DSource;
        for (std::size_t i = 0; i < steps; ++i) {
            SCircuitLocation next = previous;
            if (edge.DDestination.DX > previous.DX) {
                next.DX += stepX;
            } else if (edge.DDestination.DX < previous.DX) {
                next.DX -= stepX;
            }
            if (edge.DDestination.DY > previous.DY) {
                next.DY += stepY;
            } else if (edge.DDestination.DY < previous.DY) {
                next.DY -= stepY;       
        }
        
            result.AddEdge({previous, next});
            previous = next;
        }   
    }
    return result;
}
CCircuitTrace & CCircuitTrace::operator=(const CCircuitTrace &trace) {
    if (this != &trace) {
        DImplementation = std::make_unique<SImplementation>(*trace.DImplementation);
    }
    return *this;
}
CCircuitTrace & CCircuitTrace::operator=(CCircuitTrace &&trace) noexcept {
    if (this != &trace) {
        DImplementation = std::move(trace.DImplementation);
    }
    return *this;
}

CCircuitTrace::~CCircuitTrace() = default;

bool CCircuitTrace::ConcatenateTrace(const CCircuitTrace &trace) {
    auto current = DImplementation->DEdges;
    auto inmcoming = trace.DImplementation->DEdges;
    if (current.empty()) {
        return false; // Cannot concatenate to an empty trace
    }
    if (inmcoming.empty()) {
        return false; // Cannot concatenate an empty trace
    }
    if (current.empty()) {
       current = inmcoming; 
    } else {
        bool prepend = false;
        bool reserseIncoming = false;
        if (current.back().DDestination == inmcoming.front().DSource) {
        } else if (current.back().DDestination == inmcoming.back().DDestination) {
            reserseIncoming = true;
        } else if (current.front().DSource == inmcoming.back().DDestination) {
            prepend = true;
        } else if (current.front().DSource == inmcoming.front().DSource) {
            prepend = true;
            reserseIncoming = true;
        } else {
            return false; // No matching endpoints to concatenate
        }
        if (reserseIncoming) {
            std::vector<SCircuitTraceEdge> reversed;
            for (auto it = inmcoming.rbegin(); it != inmcoming.rend(); ++it) {
                reversed.push_back({it->DDestination, it->DSource});
            }
            inmcoming = std::move(reversed);    
        }
        if (prepend) {
            current.insert(current.begin(), inmcoming.begin(), inmcoming.end());
        } else {
            current.insert(current.end(), inmcoming.begin(), inmcoming.end());
        }
    }
    DImplementation->DEdges = std::move(current);
    DImplementation->DLocations.clear();
    for (const auto &edge : DImplementation->DEdges) {
        for (const auto &location : {edge.DSource, edge.DDestination}) {
            if (std::find(DImplementation->DLocations.begin(), DImplementation->DLocations.end(), location) == DImplementation->DLocations.end()) {
                DImplementation->DLocations.push_back(location);
            }
        }
    }
    return true;
}