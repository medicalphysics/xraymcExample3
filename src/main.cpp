/*This file is part of xraymcExample3.

xraymcExample3 is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

xraymcExample3 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with xraymcExample3. If not, see < https://www.gnu.org/licenses/>.

Copyright 2026 Erlend Andersen
*/

#include "carm.hpp"
#include "icrp110phantomreader.hpp"

#include <xraymc/beams/dxbeam.hpp>
#include <xraymc/material/material.hpp>
#include <xraymc/transport.hpp>
#include <xraymc/world/visualization/visualizeworld.hpp>
#include <xraymc/world/world.hpp>
#include <xraymc/world/worlditems/aavoxelgrid.hpp>
#include <xraymc/world/worlditems/enclosedroom.hpp>
#include <xraymc/world/worlditems/tetrahedalmesh.hpp>
#include <xraymc/world/worlditems/tetrahedalmesh/tetrahedalmeshreader.hpp>
#include <xraymc/world/worlditems/triangulatedmesh.hpp>
#include <xraymc/world/worlditems/triangulatedmesh/triangulatedmeshstlreader.hpp>
#include <xraymc/world/worlditems/triangulatedopensurface.hpp>
#include <xraymc/world/worlditems/worldbox.hpp>

constexpr int MODEL = 2;
constexpr int NSHELLS = 16;
constexpr bool FORCEINTERACTIONS = true;

using CarmType = Carm<NSHELLS, MODEL>;
using Mesh = xraymc::TriangulatedMesh<NSHELLS, MODEL>;
using VGrid = xraymc::AAVoxelGrid<NSHELLS, MODEL, 255>;
using Room = xraymc::EnclosedRoom<NSHELLS, MODEL>;
using TetMesh = xraymc::TetrahedalMesh<NSHELLS, MODEL, FORCEINTERACTIONS>;
using Box = xraymc::WorldBox<NSHELLS, MODEL>;
using Surface = xraymc::TriangulatedOpenSurface<NSHELLS, MODEL>;

using World = xraymc::World<Mesh, VGrid, Room, TetMesh, CarmType, Box, Surface>;

using Material = xraymc::Material<NSHELLS>;

// Convienence funtion to read ICRP110 voxel phantom
static VGrid getICRP110Phantom(bool female = true)
{
    auto phantom_data = female ? ICRP110PhantomReader::readFemalePhantom() : ICRP110PhantomReader::readMalePhantom();
    VGrid phantom;

    std::vector<Material> materials;
    for (auto& w : phantom_data.mediaComposition()) {
        auto mat_cand = Material::byWeight(w);
        if (mat_cand)
            materials.push_back(mat_cand.value());
        else
            throw std::runtime_error("error");
    }
    phantom.setData(phantom_data.dimensions(), phantom_data.densityData(), phantom_data.mediaData(), materials);
    phantom.setSpacing(phantom_data.spacing());
    phantom.rollAxis(2, 0);
    phantom.rollAxis(2, 1);
    phantom.flipAxis(2);
    return phantom;
}

// Convienence funtion to read ICRP 145 phantom
static TetMesh readICRP145Phantom(bool female = true)
{
    const std::string name = female ? "MRCP_AF" : "MRCP_AM";
    const std::string elefile = name + ".ele";
    const std::string nodefile = name + ".node";
    const std::string mediafile = name + "_media.dat";
    const std::string organfile = "icrp145organs.csv";

    xraymc::TetrahedalMeshReader reader(nodefile, elefile, mediafile, organfile);

    // rotating to upright position
    reader.rotate({ 0, 0, 1 }, std::numbers::pi_v<double>);
    return TetMesh { reader.data() };
}

World constructWorld()
{
    World world;
    world.reserveNumberOfItems(16);

    // Start by adding the c-arm, CarmType is a custom type that reads a stl file looking like a c-arm
    // the stl file has origin at the source position for zero primary and secondary angles
    auto& carm = world.template addItem<CarmType>({ "carm.stl" }, "C-Arm");
    // In this example we use the c-arm isocenter as world origin
    carm.translate(xraymc::vectormath::scale(carm.isoCenter(), -1.0));

    // Adding the patient table, this is a triangulated solid mesh
    xraymc::STLReader stlreader;
    stlreader.setFilePath("table.stl");
    auto& table = world.template addItem<Mesh>({ stlreader() }, "Table");
    // translate the table (in cm)
    table.translate({ 0, 0, 0 });
    const std::array<double, 6> table_aabb = table.AABB(); // get the bounding box of the table (X0 Y0 Z0 X1 Y1 Z1)
    // filling the table with carbon
    const auto carbon = Material::byZ(6).value();
    table.setMaterial(carbon);
    table.setMaterialDensity(0.2); // g/cm3

    // add a male ICRP 110 voxel phantom as patient
    auto& patient = world.addItem(getICRP110Phantom(false), "Patient");
    // translate patient to lay on table
    auto patient_aabb = patient.AABB();
    patient.translate({ table_aabb[3] - patient_aabb[3], 0, table_aabb[5] - patient_aabb[2] });

    // Make a room of 2 mm lead
    auto& room = world.template addItem<Room>("Room");
    room.setInnerRoomAABB({ -300, -200, -100, 200, 200, 180 });
    room.setWallThickness(0.2);
    const auto lead = Material::byZ(82).value();
    const auto lead_dens = xraymc::AtomHandler::Atom(82).standardDensity;
    room.setMaterial(lead, lead_dens);

    // add a concrete floor
    auto& floor = world.template addItem<Box>("Floor");
    floor.setAABB({ -299.99, -199.99, -99.99, 199.99, 199.99, -90 });
    const auto concrete = Material::byNistName("Concrete, Ordinary").value();
    const auto concrete_dens = xraymc::NISTMaterials::density("Concrete, Ordinary");
    floor.setMaterial(concrete, concrete_dens);
    auto floor_aabb = floor.AABB();

    // add a doctor
    auto& doctor = world.template addItem<TetMesh>(readICRP145Phantom(true), "Doctor");
    // make the doctor stand on the floor and beside the table
    doctor.translate({ -50, table_aabb[1] - doctor.AABB()[4] - 4, floor_aabb[5] - doctor.AABB()[2] });

    auto& nurse = world.template addItem<TetMesh>(readICRP145Phantom(false), "Nurse");
    // make the doctor stand on the floor and beside the table
    nurse.translate({ doctor.AABB()[0] - (nurse.AABB()[3] - nurse.AABB()[1]) / 2 - 10, table_aabb[1] - nurse.AABB()[4] - 4, floor_aabb[5] - nurse.AABB()[2] });

    // add ceiling shield
    stlreader.setFilePath("ceilingShield.stl");
    auto ceilingshield_tri = stlreader();
    std::for_each(std::execution::par_unseq, ceilingshield_tri.begin(), ceilingshield_tri.end(), [](auto& tri) {
        tri.rotate(std::numbers::pi_v<double> / 2, { 1, 0, 0 });
        tri.rotate(std::numbers::pi_v<double> / 2 + xraymc::DEG_TO_RAD() * 60, { 0, 0, 1 });
    });
    auto& ceilingshield = world.template addItem<Surface>({ ceilingshield_tri }, "Ceiling shield");
    ceilingshield.translate({ -15, -35, 30 });
    ceilingshield.setMaterial(lead, lead_dens);
    ceilingshield.setSurfaceThickness(0.05);

    // add table shield
    auto& table_shield = world.template addItem<Box>("Table shield");
    table_shield.setAABB({ -130, table.AABB()[1] - 1.05, floor.AABB()[5] + 5, 10, table.AABB()[1] - 1.0, table.AABB()[5] + 5.0 });
    table_shield.setMaterial(lead, lead_dens);

    // add patient blanket
    stlreader.setFilePath("blanket.stl");
    auto blanket_tri = stlreader();
    auto& blanket = world.template addItem<Surface>({ blanket_tri }, "Blanket");
    auto bc = blanket.center();
    blanket.translate(xraymc::vectormath::scale(bc, -1.0));
    blanket.translate({ -23, 0, table_aabb[5] - patient_aabb[2] + 10 });
    blanket.setMaterial(lead, lead_dens);
    blanket.setSurfaceThickness(0.05);

    // build the world
    world.build();
    return world;
}

auto getBeam(std::uint64_t n_exposures, std::uint64_t n_particles)
{
    using Beam = xraymc::DXBeam<false>;
    Beam beam;
    beam.setTubeVoltage(65);
    beam.clearTubeFiltrationMaterials();
    beam.addTubeFiltrationMaterial(13, 3.5);
    beam.addTubeFiltrationMaterial(29, 0.1);

    beam.setBeamSize(30.1 / 2, 38.7 / 2, 119.49);
    beam.setNumberOfExposures(n_exposures);
    beam.setNumberOfParticlesPerExposure(n_particles);
    beam.setDAPvalue(1);
    return beam;
}

void simulate(auto& world, auto& beam, std::uint32_t nThreads = 0)
{
    xraymc::Transport transport;
    transport.runConsole(world, beam, nThreads);
}

void visualizeWorld(const auto& world, auto* beam = nullptr, const std::string& name = "world", double zoom = 1, double maxDose = 0, bool useLogColor = false)
{
    // resolution
    constexpr int resy = 1024 * 4;
    constexpr int resx = (resy * 3) / 2;

    xraymc::VisualizeWorld viz(world);
    viz.setLowestDoseColorToWhite(false);
    viz.setDistance(1000);

    auto buffer = viz.template createBuffer<double>(resx, resy);

    // setting some colors
    std::array<std::uint8_t, 3> color;
    color = { 255, 195, 170 }; // skin
    viz.setColorOfItem(world.getItemPointerFromName("Patient"), color);
    viz.setColorOfItem(world.getItemPointerFromName("Nurse"), color);
    color = { 176, 108, 73 }; // dark skin
    viz.setColorOfItem(world.getItemPointerFromName("Doctor"), color);
    color = { 137, 207, 240 }; // baby blue
    viz.setColorOfItem(world.getItemPointerFromName("Floor"), color);
    color = { 242, 239, 223 }; // ivory white
    viz.setColorOfItem(world.getItemPointerFromName("Room"), color);
    viz.setColorOfItem(world.getItemPointerFromName("C-Arm"), color);
    color = { 55, 55, 55 }; // carbon
    viz.setColorOfItem(world.getItemPointerFromName("Table"), color);
    color = { 105, 155, 103 }; // white green
    viz.setColorOfItem(world.getItemPointerFromName("Table shield"), color);
    viz.setColorOfItem(world.getItemPointerFromName("Ceiling shield"), color);
    viz.setColorOfItem(world.getItemPointerFromName("Blanket"), color);

    std::string message;
    auto to_string = [](int n) -> std::string {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0') << n;
        return ss.str();
    };

    if (maxDose > 0) {
        viz.addColorByValueItem(world.getItemPointerFromName("Doctor"));
        viz.addColorByValueItem(world.getItemPointerFromName("Patient"));
        viz.addColorByValueItem(world.getItemPointerFromName("Nurse"));
        if (useLogColor) {
            viz.setColorByValueMinMax(maxDose / 10000.0, maxDose);
            viz.setColorByValueLogScale(true);
        } else {
            viz.setColorByValueMinMax(0, maxDose);
        }
    }

    if (beam) {
        viz.addLineProp(*beam, 95, 0.2);
    }

    constexpr int low_angle = 0;
    constexpr int high_angle = 360;
    constexpr int degStep = 30;

    viz.setAzimuthalAngleDeg(60);
    for (int i = low_angle; i < high_angle; i = i + degStep) {
        std::string fn = name + to_string(i) + ".png";
        std::cout << std::string(message.length(), ' ') << "\r";
        message = "Generating " + fn;
        std::cout << message << std::flush << "\r";
        viz.setPolarAngleDeg(i);
        viz.suggestFOV(zoom);
        viz.generate(world, buffer);
        viz.savePNG(fn, buffer);
    }

    std::cout << std::string(message.length(), ' ') << "\r";
}

int main()
{
    // create world
    auto world = constructWorld();

    // add a beam
    xraymc::DXBeam<> beam;
    beam.setTubeVoltage(65);
    beam.clearTubeFiltrationMaterials();
    beam.addTubeFiltrationMaterial(13, 3.5);
    beam.addTubeFiltrationMaterial(29, 0.1);

    beam.setCollimationHalfAnglesDeg(2, 2);
    beam.setNumberOfExposures(100);
    beam.setNumberOfParticlesPerExposure(1E6);
    beam.setDAPvalue(1);

    auto carm = std::get_if<CarmType>(world.getItemPointerFromName("C-Arm"));
    if (carm) {
        carm->setPrimaryAngleDeg(10);
        carm->setSecondaryAngleDeg(15);
        beam.setPosition(carm->beamSourcePos());
        beam.setDirectionCosines(carm->beamCosines());
    }
    world.build(); // build since geometry has changed
    visualizeWorld(world, &beam, "show", 1.5);

    // simulate
    xraymc::Transport transport;
    transport.runConsole(world, beam);

    auto doctor = std::get_if<TetMesh>(world.getItemPointerFromName("Doctor"));
    if (doctor) {
        double max_dose = 0;
        const auto& outerTetIdx = doctor->outerContourTetrahedronIndices();
        for (std::size_t i = 0; i < outerTetIdx.size(); ++i)
            max_dose = std::max(max_dose, doctor->doseScored(outerTetIdx[i]).dose());
        visualizeWorld(world, &beam, "doseLog", 1.5, max_dose / 10.0, true);
        visualizeWorld(world, &beam, "dose", 1.5, max_dose / 10.0, false);
    }
    return EXIT_SUCCESS;
}