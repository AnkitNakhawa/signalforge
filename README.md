# signalforge

Signalforge is a high-performance C++ execution and order book core
designed for AI-driven trading strategy research and testing.

## Goals
- Deterministic L2 order book
- Event-driven replay using real exchange data
- Execution simulation for strategy evaluation
- Clean separation between execution core and strategy logic

## Non-Goals
- Colocated HFT
- L3 order reconstruction
- Real capital deployment

## Architecture
Signalforge is built as an execution-grade data plane that can be driven
by higher-level strategy runtimes (Python, agents, LLM-generated logic).

## Status
Early development. Core order book API and build system in place.
