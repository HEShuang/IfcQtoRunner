# IfcQtoRunner

[![Lifecycle: In Development](https://img.shields.io/badge/lifecycle-in%20development-orange)]()
[![License: MIT](https://img.shields.io/badge/license-MIT-blue)]()

---

## Overview

**IfcQtoRunner** is a C++/Qt application for working with IFC files. It allows you to:

* **Import** IFC files using IfcOpenShell
* **Extract and display** the project structure (✅ Done)
* **Visualize** geometry via the IfcOpenShell Python API (⚙️ Ongoing)
* **Perform** quantity take‑offs (🚧 Upcoming)

## Purpose

* **Support evolving IFC standards** — adapt easily to new schema versions (IFC2x3, IFC4.x, future IFC5 …)
* **Integrate Python in C++** — leverage IfcOpenShell’s Python API from native code
* **Use modern C++** — C++17/20 features, move semantics, efficient containers
* **Enable fast quantity take‑off** — rapid standard IFC quantity computations for AEC workflows

## Features

* **Structure Preview**: Browse building hierarchies down to storeys and elements
* **Geometry Viewer**: In‑progress 3D rendering powered by IfcOpenShell’s Python API
* **Quantity Take‑off**: Upcoming feature for automated count, area, and volume reporting

## License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE) for details.
