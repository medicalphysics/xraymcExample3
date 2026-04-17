# xraymcExample3

## About

This example demonstrates how to use the [XRayMClib](https://github.com/medicalphysics/XRayMClib) C++ library to simulate radiation transport in a full interventional radiology lab. The simulation models scattered X-ray dose to staff and patient during a fluoroscopic procedure, using Monte Carlo photon transport with realistic geometry and physics.

The scene includes:
- A C-arm fluoroscopy unit (triangulated STL mesh) with configurable primary and secondary angles
- A patient (ICRP 110 voxel phantom, male) lying on a carbon-fibre table
- A doctor and nurse (ICRP 145 tetrahedral mesh phantoms) standing beside the table
- A lead-lined room with a concrete floor
- Radiation protection equipment: ceiling-mounted lead shield, table-side lead shield, and a patient lead blanket
- A DX beam with realistic tube voltage, filtration (Al + Cu), and DAP-based normalization. The beam is set to 65 kVp with 3.5 mm Al and 0.1 mm Cu filtration, a 2×2 degree half-angle collimation, and a 1 mGy·cm² DAP normalization. The C-arm is positioned at 10° primary and 15° secondary angle.

After simulation, dose results are saved to disk and visualized as a series of PNG images rendered from multiple angles using path tracing.


## Project structure

```
xraymcExample3/
├── src/
│   ├── main.cpp                   # Simulation setup, run, and visualization
│   └── icrp110phantomreader.cpp   # Reader for ICRP 110 voxel phantom data
├── include/
│   ├── carm.hpp                   # C-arm world item wrapping a triangulated mesh
│   └── icrp110phantomreader.hpp
├── data/
│   ├── ICRP110/AF/                # ICRP 110 female voxel phantom (LFS)
│   ├── ICRP110/AM/                # ICRP 110 male voxel phantom (LFS)
│   ├── ICRP145/MRCP_AF/           # ICRP 145 female tetrahedral phantom (LFS)
│   ├── ICRP145/MRCP_AM/           # ICRP 145 male tetrahedral phantom (LFS)
│   └── STL/                       # STL meshes: carm, table, blanket, ceilingShield
├── CMakeLists.txt
└── README.md
```

Phantom data files are stored using Git LFS because of their size.

## Requirements

- **C++ compiler** with C++23 support: MSVC ≥ 16.8, Clang ≥ 13, or GCC ≥ 11
- **CMake** ≥ 3.20
- **Git** with **Git LFS** (required to download phantom data)
- Internet access at configure time (CMake downloads XRayMClib and EPICS physics data automatically)

## Building and running

### Clone the repository

Git LFS is required to download the phantom data files. Make sure it is installed before cloning.

```bash
git clone https://github.com/medicalphysics/xraymcExample3.git
cd xraymcExample3
```

### Configure

CMake will automatically fetch XRayMClib from GitHub and download EPICS photon cross-section data from the IAEA during this step.

```bash
cmake -S . -B build
```

### Build

```bash
cmake --build build --config Release --target xraymcexample3
```

### Run

Navigate to the output directory and run the executable:

```bash
# Linux / macOS
./build/xraymcexample3

# Windows
build\Release\xraymcexample3.exe
```

The program checks for an existing `savefile.xr` in the working directory. If not found, it runs the full simulation (which may take several minutes depending on hardware) and saves the result. It then loads the result and generates PNG images of the world geometry and dose distribution from multiple viewing angles.

To force a new simulation even if a save file exists, set `force_new_simulation = true` in `main()` in [src/main.cpp](src/main.cpp).

## Output

The program produces sets of PNG images in the working directory:

| File pattern | Content |
|---|---|
| `world000.png` – `world330.png` | Path-traced view of the geometry from 12 angles |
| `dose000.png` – `dose330.png` | Linear dose colour map on phantoms |
| `doseLog000.png` – `doseLog330.png` | Logarithmic dose colour map on phantoms |

Images are rendered at 1536×1024 pixels from a camera distance of 1000 cm with a 60° azimuthal angle, stepping 30° around the scene.

## CMake options

Two options are set in [CMakeLists.txt](CMakeLists.txt) before the FetchContent call and control how XRayMClib is built:

| Option | Default | Description |
|---|---|---|
| `XRAYMCLIB_EPICS_DOWNLOAD` | `ON` | Automatically download EPICS 2023 physics data at configure time |
| `XRAYMCLIB_USE_LOADPNG` | `ON` | Enable PNG image output in the visualizer |

## License

GPL-3.0 — see [LICENSE](LICENSE).