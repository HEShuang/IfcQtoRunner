# IfcQtoRunner

[![Lifecycle: In Development](https://img.shields.io/badge/lifecycle-in%20development-orange)]()
[![License: MIT](https://img.shields.io/badge/license-MIT-blue)]()

---

## Overview

**IfcQtoRunner** is a C++/Qt application for working with IFC files. It allows you to:

* **Import** IFC files using IfcOpenShell
* **Extract and display** the project structure (✅ Done)
* **Visualize** 3D model while importing (✅ Done)
* **Explore** each element (⚙️ Ongoing)
* **Perform** quantity take‑offs (🚧 Upcoming)
  
## Purpose

* **Support evolving IFC standards** — adapt easily to new schema versions (IFC2x3, IFC4.x, future IFC5 …)
* **Use modern C++** — C++17/20 features, multi-threading, move semantics, efficient containers
* **Enable fast quantity take‑off** — rapid standard IFC quantity computations for AEC workflows

## Features

* **Structure Preview**: Browse building hierarchies down to storeys and elements
* **Geometry Viewer**: 3D rendering powered Qt OpenGL
* **Progressive Rendering**: Rendering while importing
* **Quantity Take‑off**: Ongoing feature for automated count, area, and volume reporting

![demo](https://github.com/user-attachments/assets/b5bfdde2-cf5b-4ea9-a11c-c24c963e53bb)

## License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE) for details.
