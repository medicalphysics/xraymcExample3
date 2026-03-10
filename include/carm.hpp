/*This file is part of xraymcCarm.

xraymcCarm is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

xraymcCarm is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with xraymcCarm. If not, see < https://www.gnu.org/licenses/>.

Copyright 2024 Erlend Andersen
*/

#pragma once

#include <xraymc/world/worlditems/triangulatedmesh.hpp>

#include <array>
#include <string>

template <int NMaterialShells = 5, int LOWENERGYCORRECTION = 2>
class Carm {
public:
    Carm(const std::string& path, std::size_t max_three_dept = 4)
        : m_mesh(path, max_three_dept)
    {
        const auto carbon = xraymc::Material<NMaterialShells>::byZ(6).value();
        const auto carbon_dens = xraymc::AtomHandler::Atom(6).standardDensity;
        m_mesh.setMaterial(carbon, carbon_dens);
    }

    void scale(double s)
    {
        m_mesh.scale(s);
        for (std::size_t i = 0; i < 3; ++i) {
            m_source_pos[i] *= s;
            m_isocenter[i] *= s;
        }
    }

    void rotate(const std::array<double, 3>& axis, double angle)
    {
        m_mesh.rotate(angle, axis);

        m_source_pos = xraymc::vectormath::rotate(m_source_pos, axis, angle);
        m_isocenter = xraymc::vectormath::rotate(m_isocenter, axis, angle);
        m_xAxis = xraymc::vectormath::rotate(m_xAxis, axis, angle);
        m_yAxis = xraymc::vectormath::rotate(m_yAxis, axis, angle);
        m_beam_cosines[0] = xraymc::vectormath::rotate(m_beam_cosines[0], axis, angle);
        m_beam_cosines[1] = xraymc::vectormath::rotate(m_beam_cosines[1], axis, angle);
    }

    void translate(const std::array<double, 3>& translation)
    {
        m_mesh.translate(translation);
        m_source_pos = xraymc::vectormath::add(m_source_pos, translation);
        m_isocenter = xraymc::vectormath::add(m_isocenter, translation);
    }

    void setPrimaryAngleDeg(double angle)
    {
        setPrimaryAngle(angle * xraymc::DEG_TO_RAD<>());
    }
    void setSecondaryAngleDeg(double angle)
    {
        setSecondaryAngle(angle * xraymc::DEG_TO_RAD<>());
    }

    void setPrimaryAngle(double new_angle)
    {
        const double angle = new_angle - m_primary_angle;
        m_primary_angle = new_angle;

        m_mesh.rotate(angle, m_xAxis, m_isocenter);

        auto d = xraymc::vectormath::subtract(m_source_pos, m_isocenter);
        auto d_fin = xraymc::vectormath::rotate(d, m_xAxis, angle);
        m_source_pos = xraymc::vectormath::add(m_isocenter, d_fin);
        m_beam_cosines[0] = xraymc::vectormath::rotate(m_beam_cosines[0], m_xAxis, angle);
        m_beam_cosines[1] = xraymc::vectormath::rotate(m_beam_cosines[1], m_xAxis, angle);
    }
    void setSecondaryAngle(double new_angle)
    {
        const double angle = new_angle - m_secondary_angle;
        m_secondary_angle = new_angle;

        m_mesh.rotate(angle, m_yAxis, m_isocenter);

        auto d = xraymc::vectormath::subtract(m_source_pos, m_isocenter);
        auto d_fin = xraymc::vectormath::rotate(d, m_yAxis, angle);
        m_source_pos = xraymc::vectormath::add(m_isocenter, d_fin);
        m_beam_cosines[0] = xraymc::vectormath::rotate(m_beam_cosines[0], m_yAxis, angle);
        m_beam_cosines[1] = xraymc::vectormath::rotate(m_beam_cosines[1], m_yAxis, angle);
    }
    const std::array<double, 3>& center() const
    {
        return m_isocenter;
    }
    std::array<double, 6> AABB() const
    {
        return m_mesh.AABB();
    }
    xraymc::WorldIntersectionResult intersect(const xraymc::ParticleType auto& p) const
    {
        return m_mesh.intersect(p);
    }
    template <typename U>
    xraymc::VisualizationIntersectionResult<U> intersectVisualization(const xraymc::ParticleType auto& p) const
    {
        return m_mesh.template intersectVisualization<U>(p);
    }
    auto energyScored(std::size_t index = 0)
    {
        return m_mesh.energyScored(index);
    }
    auto doseScored(std::size_t index = 0) const
    {
        return m_mesh.doseScored(index);
    }
    void clearDoseScored()
    {
        m_mesh.clearDoseScored();
    }
    void clearEnergyScored()
    {
        m_mesh.clearEnergyScored();
    }
    void addEnergyScoredToDoseScore(double factor)
    {
        m_mesh.addEnergyScoredToDoseScore(factor);
    }
    void transport(xraymc::ParticleType auto& p, xraymc::RandomState& state)
    {
        m_mesh.transport(p, state);
    }

    std::array<double, 3> beamSourcePos() const
    {
        return m_source_pos;
    }
    const std::array<std::array<double, 3>, 2>& beamCosines() const
    {
        return m_beam_cosines;
    }

    std::array<double, 3> isoCenter() const
    {
        return m_isocenter;
    }

private:
    xraymc::TriangulatedMesh<NMaterialShells, LOWENERGYCORRECTION> m_mesh;
    std::array<double, 3> m_source_pos = { 0, 0, 0 };
    std::array<double, 3> m_isocenter = { 0, 0, 55 };
    std::array<double, 3> m_xAxis = { 1, 0, 0 };
    std::array<double, 3> m_yAxis = { 0, 1, 0 };
    std::array<std::array<double, 3>, 2> m_beam_cosines = { { { 1, 0, 0 }, { 0, 1, 0 } } };
    double m_primary_angle = 0;
    double m_secondary_angle = 0;
};
