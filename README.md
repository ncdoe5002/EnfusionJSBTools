# JSBSim Enfusion Utils (JSBSimXMLParser)

A lightweight, high-performance C++ utility designed to parse JSBSim aircraft XML models and generate native Bohemia Interactive Enfusion engine configuration files (`.conf` and `.conf.meta`) for Arma Reforger.

## Features

- **Metrics & Geometry Conversion**: Extracts dimensions, wing areas, chord lengths, tail arms, and incidence angles into `<Prefix>_AircraftGeometry.conf`.
- **Flight Control System (FCS)**: Extracts channels, summer components, switches, kinematics, and limits into `<Prefix>_FlightControls.conf`.
- **Aerodynamics & Lookup Tables**: Parses complex JSBSim aerodynamic force/moment functions and transforms 1D, 2D, and 3D lookup tables into Enfusion curve/spline definitions in `<Prefix>_Aerodynamics.conf`.
- **Enfusion Metadata Generation**: Automatically generates valid Enfusion `.conf.meta` resource files with unique GUIDs.
- **Fast & Zero External Dependencies**: Written in modern C++17 with header-only parsers.

---

## Project Structure

```
JSBSimEnfusionUtils/
├── src/
│   ├── AerodynamicsParser.hpp   # Aerodynamic function and table parser
│   ├── FlightControlParser.hpp  # FCS channel and component parser
│   ├── GuidGenerator.hpp        # Enfusion GUID generator
│   ├── MetricsParser.hpp        # Geometry and metrics parser
│   ├── XmlParser.hpp            # Lightweight XML tree parser
│   └── main.cpp                 # CLI entry point and argument handling
├── build.bat                    # Windows MSVC batch build script
├── run_parser.bat               # Wrapper script for batch conversions
├── CMakeLists.txt               # CMake configuration
├── .gitignore                   # Standard Git ignore rules
└── README.md
```

---

## Building

### Option 1: Using `build.bat` (Recommended on Windows)

Make sure you have Visual Studio 2022 (Community, Professional, or Build Tools) installed:

```cmd
build.bat
```

This compiles `JSBSimXMLParser.exe` using `cl.exe` with C++17 and optimization enabled.

### Option 2: Using CMake

```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

---

## Usage

### Command Line Interface

```bash
JSBSimXMLParser.exe --xml <path_to_jsbsim_xml> --out <output_directory> [options]
```

### Options

| Flag | Description | Default |
|------|-------------|---------|
| `--xml <path>` | Path to input JSBSim XML aircraft file | *(Required)* |
| `--out <dir>` | Output folder for generated Enfusion `.conf` files | *(Required)* |
| `--prefix <name>` | Prefix applied to output file names | `Aircraft` |
| `--aircraft <name>` | Aircraft display name | Derived from XML |
| `--parse-aero <0\|1>` | Enable/disable aerodynamics parsing | `1` |
| `--parse-fcs <0\|1>` | Enable/disable flight control parsing | `1` |
| `--parse-metrics <0\|1>` | Enable/disable aircraft metrics parsing | `1` |
| `--help` | Show usage and flag reference | |

### Example

```cmd
JSBSimXMLParser.exe --xml "C:\Aircraft\b17.xml" --out ".\output" --prefix "B17" --aircraft "B-17 Flying Fortress"
```

Generated files:
- `output/B17_AircraftGeometry.conf` & `.meta`
- `output/B17_FlightControls.conf` & `.meta`
- `output/B17_Aerodynamics.conf` & `.meta`

### Using `run_parser.bat`

```cmd
run_parser.bat <xml_path> <output_dir> <prefix> <aircraft_name> <parse_aero> <parse_fcs> <parse_metrics>
```

---

## License

MIT License. See project headers for attribution.
