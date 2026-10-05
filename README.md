# JSBEnfusionTools

A suite of developer tools and utilities designed to integrate **JSBSim** flight dynamics models with Bohemia Interactive's **Enfusion Engine** for Arma Reforger.

## Tools in This Repository

| Tool | Directory | Language | Description |
|------|-----------|----------|-------------|
| **XMLParser** | [`XMLParser/`](./XMLParser) | C++17 | Parses JSBSim aircraft XML models into native Enfusion configuration files (`.conf` and GUID-stamped `.conf.meta`). |

*(Additional flight modeling, telemetry, and conversion tools will be placed in their own subdirectories as they are developed.)*

---

## Repository Structure

```
JSBEnfusionTools/
├── XMLParser/                   # JSBSim XML -> Enfusion Config Converter
│   ├── src/                     # C++17 parser source files
│   ├── CMakeLists.txt           # CMake build configuration for parser
│   ├── build.bat                # Standalone MSVC build script
│   ├── run_parser.bat           # Command-line execution wrapper
│   └── README.md                # XMLParser detailed documentation
├── .gitignore                   # Repository-wide ignore rules
└── README.md                    # Main repository overview
```

---

## Tool Documentation

- For building, running, and configuring the XML parser, see the [XMLParser README](./XMLParser/README.md).

---

## License

MIT License.
