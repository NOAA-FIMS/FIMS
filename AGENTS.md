# Instructions for AI Coding Agents

These instructions apply to the entire FIMS repository.

Before proposing or making changes, read and follow the [FIMS AI-Assisted Development Guidelines](AI_GUIDELINES.md) and the relevant sections of [CONTRIBUTING.md](CONTRIBUTING.md). The AI guidelines are mandatory development policy, not optional suggestions.

In particular:

- Understand the requested behavior, inspect the nearest existing implementation, trace affected interfaces, and review relevant tests before editing.
- Preserve scientific meaning, statistical formulations, units, scales, dimensions, indexing, invariants, numerical behavior, derivative behavior, and public API contracts.
- Follow established FIMS architecture and terminology across R, Rcpp, C++, TMB, serialization, and user-facing interfaces.
- Do not silently reinterpret data, redesign interfaces, introduce unrelated refactors, or invent repository APIs and conventions.
- Resolve routine implementation details from repository evidence. Surface unresolved scientific, statistical, architectural, or API ambiguity instead of guessing.
- Keep changes focused; document non-obvious intent and validate behavior with tests appropriate to the risk.
- Follow the repository's R and Google C++ style requirements and all standard contributor checks that apply to the change.

Explicit task-specific direction from a FIMS maintainer governs when it conflicts with these general instructions, but the conflict and its implications must be made clear.
