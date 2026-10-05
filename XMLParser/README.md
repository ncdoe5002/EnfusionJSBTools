# XMLParser (JSBSim to Enfusion Config Converter)

A lightweight, high-performance C++17 utility designed to parse JSBSim aircraft XML models and generate native Bohemia Interactive Enfusion engine configuration files (`.conf` and `.conf.meta`) for Arma Reforger.

## Features

- **Metrics & Geometry Conversion**: Extracts dimensions, wing areas, chord lengths, tail arms, and incidence angles into `<Prefix>_AircraftGeometry.conf`.
- **Flight Control System (FCS)**: Extracts channels, summer components, switches, kinematics, and limits into `<Prefix>_FlightControls.conf`.
- **Aerodynamics & Lookup Tables**: Parses complex JSBSim aerodynamic force/moment functions and transforms 1D, 2D, and 3D lookup tables into Enfusion curve/spline definitions in `<Prefix>_Aerodynamics.conf`.
- **Enfusion Metadata Generation**: Automatically generates valid Enfusion `.conf.meta` resource files with unique GUIDs.
- **Fast & Zero External Dependencies**: Written in modern C++17 with header-only parsers.

---

## Building

### Option 1: Using `build.bat` (Recommended on Windows)

Make sure Visual Studio 2022 (Community, Professional, or Build Tools) is installed:

```cmd
build.bat
```

This compiles `JSBSimXMLParser.exe` using MSVC `cl.exe` with C++17 optimization.

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
