#include "CircuitNet.h"
#include <algorithm>
#include <utility>
#include <cmath>

struct CCircuitNet::SImplementation {
    TCircuitName DName;
    std::vector<SCircuitLocation> DPins;
    std::vector<CCircuitTrace> DTraces;
};

CCircuitNet::CCircuitNet()
    : DImplementation(std::make_unique<SImplementation>()) {}
CCircuitNet::CCircuitNet(const CCircuitNet &net)
    : DImplementation(std::make_unique<SImplementation>(*net.DImplementation)) {}
CCircuitNet::CCircuitNet(CCircuitNet &&net) noexcept
    : DImplementation(std::move(net.DImplementation)) {}
CCircuitNet::~CCircuitNet() = default;
CCircuitNet & CCircuitNet::operator=(const CCircuitNet &net) {
    if (this != &net) {
        DImplementation = std::make_unique<SImplementation>(*net.DImplementation);
    }
    return *this;
}
CCircuitNet & CCircuitNet::operator=(CCircuitNet &&net) noexcept {
    if (this != &net) {
        DImplementation = std::move(net.DImplementation);
    }
    return *this;
}
TCircuitName CCircuitNet::Name() const {
    return DImplementation->DName;
}
TCircuitName CCircuitNet::Name(const TCircuitName &name)
{
    if (!name.empty()) {
        DImplementation->DName = name;
    }
    return DImplementation->DName;
}

std::size_t CCircuitNet::PinCount() const {
    return DImplementation->DPins.size();
}
bool CCircuitNet::PinLocation(std::size_t index, SCircuitLocation &location) const {
    if (index >= DImplementation->DPins.size()) {
        return false;
    }
    location = DImplementation->DPins[index];
    return true;
}

bool CCircuitNet::AddPinLocation(const SCircuitLocation &location) {
    if (std::find(DImplementation->DPins.begin(), DImplementation->DPins.end(), location) != DImplementation->DPins.end()) {
        return false; // Pin already exists
    }
    DImplementation->DPins.push_back(location);
    return true;
}
std::size_t CCircuitNet::TraceCount() const {
    return DImplementation->DTraces.size();
}
bool CCircuitNet::Trace(std::size_t index, CCircuitTrace &trace) const {
    if (index >= DImplementation->DTraces.size()) {
        return false;
    }
    trace = DImplementation->DTraces[index];
    return true;
}
bool CCircuitNet::AddTrace(const CCircuitTrace &trace) {
    if (trace.Name() != DImplementation->DName) {
        return false; // Trace name does not match net name
    }
    for (const auto &existing : DImplementation->DTraces) {
        if (existing.EdgeCount() != trace.EdgeCount()) {
            continue; // Different number of edges, cannot be the same trace
        }
        bool same = true;
        for (std::size_t i = 0; i < trace.EdgeCount(); ++i) {
            SCircuitTraceEdge a, b;
            existing.Edge(i, a);
            trace.Edge(i, b);

            if (a.DSource != b.DSource ||
                a.DDestination != b.DDestination) {
                same = false;
                break;
            }
        }
            if (same) {
                return false; // Found a trace with the same edges, cannot add duplicate
            }
        }
        DImplementation->DTraces.push_back(trace);
        return true;
    }
bool CCircuitNet::IsBroken() const {
    const auto &trace = DImplementation->DTraces;
    if (trace.empty()) {
        return true; // No traces means the net is broken
    }
    std::vector<SCircuitLocation> Locations;
    std::vector<std::pair<std::size_t, std::size_t>> Links;
    auto getIndex = [&](const SCircuitLocation &location) {
        auto it = std::find(Locations.begin(), Locations.end(), location);
        if (it == Locations.end()) {
           Locations.push_back(location);
           return Locations.size() - 1;
        }
        return static_cast<std::size_t>(it - Locations.begin());
    };

    for (const auto &trace : trace) {
        if (trace.IsBroken()) {
            return true; // Found a broken trace
        }
        for (std::size_t i = 0; i < trace.EdgeCount(); ++i) {
            SCircuitTraceEdge edge;
            trace.Edge(i, edge);
            Links.emplace_back(getIndex(edge.DSource), getIndex(edge.DDestination));    
        }
    }
    if (DImplementation->DPins.empty()) {
        return false; // No pins means the net is broken
    }
    std::vector<std::vector<std::size_t>> adjacency(Locations.size());
    for (const auto &[a, b] : Links) {
        adjacency[a].push_back(b);
        adjacency[b].push_back(a);
    }
    auto firstPin = std::find(
        Locations.begin(), Locations.end(), DImplementation->DPins.front());
    if (firstPin == Locations.end()) {
        return true; // First pin not found in locations, net is broken
    }
    std::vector<bool> visited(Locations.size(), false);
    std::vector<std::size_t> pending;
    const auto start = static_cast<std::size_t>(firstPin - Locations.begin());
    visited[start] = true;
    pending.push_back(start);
    for (std::size_t i = 0; i < pending.size(); ++i) {
        for (const auto next : adjacency[pending[i]]) {
            if (!visited[next]) {
                visited[next] = true;
                pending.push_back(next);
            }
        }
    }
        for (const auto &pin : DImplementation->DPins) {
            auto it = std::find(Locations.begin(), Locations.end(), pin);
            if (it == Locations.end()) {
                return true; // Pin not found in locations, net is broken
            }
            const auto index = static_cast<std::size_t>(it - Locations.begin());
            if (!visited[index]) {
                return true; // Pin not reachable, net is broken
            }
        }
    return false; // All pins are reachable, net is not broken
}
bool CCircuitNet::Fix() {
    if (DImplementation->DTraces.empty()) {
        return false; // No traces to fix
    }
    bool fixedAll = true;
    for (auto &trace : DImplementation->DTraces) {
        if (trace.IsBroken() && !trace.Fix()) {
            fixedAll = false; // Found a trace that could not be fixed
        }
    }
    return fixedAll && !IsBroken();
}
bool CCircuitNet::ElaborateTraces() {
    if (DImplementation->DTraces.empty()) {
        return false; // No traces to elaborate
    }
    for (auto &trace : DImplementation->DTraces) {
        CCircuitTrace expanded = trace.Elaborate();
        trace = std::move(expanded);
    }
    return true;
}
bool CCircuitNet::SimplifyTraces() {
    const auto &oldTraces = DImplementation->DTraces;
    if (oldTraces.empty()) {
        return false; // No traces to simplify
    }
    std::vector<SCircuitTraceEdge> segments;
    for (const auto &trace : oldTraces) {
        for (std::size_t i = 0; i < trace.EdgeCount(); ++i) {
            SCircuitTraceEdge edge;
            trace.Edge(i, edge);
            segments.push_back(edge);
        }
    }
    std::vector<SCircuitLocation> locations;
    std::vector<std::pair<std::size_t, std::size_t>> links;
    auto getIndex = [&locations](const SCircuitLocation &location) {
        auto it = std::find(locations.begin(), locations.end(), location);
        if (it == locations.end()) {
            locations.push_back(location);
            return locations.size() - 1;
        }
        return static_cast<std::size_t>(it - locations.begin());
    };
    auto addLink = [&](const SCircuitLocation &a, const SCircuitLocation &b) {
       if (a == b) {
            return; // Ignore self-links
        }
        const auto ai = getIndex(a);
        const auto bi = getIndex(b);
        for (const auto &link : links) {
            if ((link.first == ai && link.second == bi) || (link.first == bi && link.second == ai)) {
                return; // Link already exists
            }
        }
        links.emplace_back(ai, bi);
    };  
    for (std::size_t i = 0; i < segments.size(); ++i) {
        const auto &edge = segments[i];
        const long double ax = edge.DSource.DX;
        const long double ay = edge.DSource.DY;
        const long double rx = static_cast<long double>(edge.DDestination.DX) - ax;
        const long double ry = static_cast<long double>(edge.DDestination.DY) - ay;
        std::vector<std::pair<long double,SCircuitLocation>> cuts;
        cuts.push_back({0, edge.DSource});
        cuts.push_back({1, edge.DDestination});
        for (std::size_t j = 0; j < segments.size(); ++j) {
            if (i == j) {
                continue; // Skip self
            }   
        const auto &other = segments[j];
        const long double cx = other.DSource.DX;
        const long double cy = other.DSource.DY;
        const long double sx = static_cast<long double>(other.DDestination.DX) - cx;
        const long double sy = static_cast<long double>(other.DDestination.DY) - cy;
        const long double denominator = rx * sy - ry * sx;
        const long double qx = cx - ax;
        const long double qy = cy - ay;

                if (denominator == 0) {
                        // 平行时，如果共线，就把对方落在线段上的端点作为切分点
                        if (qx * ry - qy * rx == 0) {
                            auto addCollinearCut = [&](const SCircuitLocation &point) {
                                long double t;

                        if (rx != 0) {
                            t = (static_cast<long double>(point.DX) - ax) / rx;
                        } else if (ry != 0) {
                            t = (static_cast<long double>(point.DY) - ay) / ry;
                        } else {
                            return;
                        }

                        if (t >= 0 && t <= 1) {
                            cuts.push_back({t, point});
                        }
                    };

                    addCollinearCut(other.DSource);
                    addCollinearCut(other.DDestination);
                }
                continue;
            }

        const long double t = (qx * sy - qy * sx) / denominator;
        const long double u = (qx * ry - qy * rx) / denominator;

            if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
                SCircuitLocation crossing{
                    static_cast<std::size_t>(std::round(ax + t * rx)),
                    static_cast<std::size_t>(std::round(ay + t * ry))
                };
                cuts.push_back({t, crossing});
            }
        }
        std::sort(cuts.begin(), cuts.end(), [](const auto &a, const auto &b) {
            return a.first < b.first;
        });
        for (std::size_t j = 1; j < cuts.size(); ++j) {
            addLink(cuts[j - 1].second, cuts[j].second);
        }
    }
    std::vector<std::vector<std::size_t>> adjacency(locations.size());
    for (const auto &[a, b] : links) {
        adjacency[a].push_back(b);
        adjacency[b].push_back(a);
    }
    auto isTerminal = [&](std::size_t index) {
        return adjacency[index].size() != 2 || 
            std::find(DImplementation->DPins.begin(), DImplementation->DPins.end(), locations[index]) != DImplementation->DPins.end();
    };
    std::vector<bool> used(links.size(), false);
    std::vector<CCircuitTrace> simplified;
    for (std::size_t start = 0; start < locations.size(); ++start) {
        if (!isTerminal(start)) {
            continue; // Not a terminal node
        }
        for (std::size_t first = 0; first < links.size(); ++first) {
            if (used[first] ||
    (links[first].first != start && links[first].second != start)) {
                    continue; // Link already used or does not start from this terminal
                }
                CCircuitTrace path;
                path.Name(DImplementation->DName);
                std::size_t current = start;
                std::size_t linkIndex = first;
                while (true) {
                    used[linkIndex] = true;
                    const auto &link = links[linkIndex].first == current ? links[linkIndex].second : links[linkIndex].first;
                    path.AddEdge({locations[current], locations[link]});
                    current = link;
                    if (isTerminal(current)) {
                        break; // Reached another terminal
                    }
                    bool foundNext = false;
                    for (std::size_t k = 0; k < links.size(); ++k) {
                       if (!used[k] &&
                            (links[k].first == current || links[k].second == current)) {
                            linkIndex = k;
                            foundNext = true;
                            break;
                        }
                    }
                    if (!foundNext) {
                        break;
                    }
                }
                simplified.push_back(path.Simplify());
            }
        }
        for (std::size_t i = 0; i < links.size(); ++i) {
            if (!used[i]) {
                CCircuitTrace path;
                path.Name(DImplementation->DName);
                path.AddEdge({locations[links[i].first], locations[links[i].second]});
                simplified.push_back(path);
                }
            }
            DImplementation->DTraces = std::move(simplified);
        
            return !DImplementation->DTraces.empty(); // Return true if any traces were simplified
        }
