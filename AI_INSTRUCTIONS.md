# AI Instructions

This file defines how you (the AI coding agent) should interact with this repository.  
Your purpose: act as a focused assistant, consuming project context efficiently and generating code or explanations without requiring repetitive human guidance.

---

## 1. Operating Model & Preflight
- Role: Act as a focused assistant making surgical edits and additions. Keep changes minimal and precise; answer questions clearly when asked. When unsure, present options with pros/cons.
- How to use this file: Treat it as the table of contents. Use the guide index to locate authoritative context for the task. Default to slightly more context if unsure.
- Preflight Checklist (must do before any edit or significant decision or answer given):
  - Read this file and the relevant linked guides for the task (architecture/layout, modules/ownership, state/FP, hardware/I2S, library APIs).
  - Identify what applies; do not proceed until preflight is complete.
  - Every edit/PR must include a one‑liner: “Guides consulted: <list> (sections)”.

---

## 2. Guide Index (single source of truth)
- `README.md` — project overview and intent.
  - Consult when: scoping tasks; validating high‑level goals.
- `ai/code_structure.md` — organization, module boundaries, ownership, build layout.
  - Consult when: adding/moving files; creating/updating modules; choosing folders; introducing shared helpers; header vs cpp.
- `ai/functional_programming.md` — state containers, pure transitions, ISR boundary patterns.
  - Consult when: defining/updating state; designing update loops; crossing ISR boundaries; adding mappers/query helpers.
- `ai/hardware.md` and `ai/library_reference/` — hardware/I2S roles, pins/clocks, relevant library APIs.
  - Consult when: changing pins/roles/clocks; touching I2S buffer/timing; using driver calls; ADC/attenuation.

---

## 3. Navigation Strategy
- Start from this file.  
- ALWAYS complete the preflight and consult relevant guides before edits.  
- Prefer comprehensive sources (e.g. project summaries).  
- Skip clearly irrelevant files only after preflight confirms they are not needed.  

---

## 4. Preferences
- Keep outputs **concise, explicit, and technically precise**.  
- Default to industry best practices (naming, error handling, modularity).  
- Where assumptions are necessary, clearly mark them.  
- Always ground responses in available repo context before guessing.  
- Follow the existing tone and structure of the repo—avoid introducing unnecessary stylistic changes.  

---

## 5. Documentation Rules (to avoid code drift)
- Do not reference specific source file paths, classes, or function names in `ai/` docs or this file.  
- Prefer generic examples that convey patterns rather than concrete project identifiers.  
- If you must include code snippets, make them self-contained and decoupled from current filenames/symbols.

---

## 6. Priority of Sources (authoritative order)
1) This file and the linked guides in `ai/` (this repo’s source of truth)
2) Project configuration (`src/config/constants.h`)
3) Current codebase patterns
4) General/industry best practices

If a lower‑priority source conflicts with a higher one, follow the higher one.

---

## 7. Conflict Handling
- If existing code diverges from the guides, treat it as code drift. Prefer aligning code to the guides.
- If a guide seems outdated or insufficient, pause and propose a minimal guide update; implement only after the guide is updated.

---

## 8. Intentional Divergence Protocol
- To change established patterns, update the relevant guide(s) first.
- Reference the guide change in the edit/PR summary: “Following updated functional_programming §X: <new approach>.”

---

## 9. Status/PR Note Requirement
Every edit/PR must state which guides and sections were consulted (see §1 Preflight). This enforces the preflight and helps reviewers verify source‑of‑truth compliance.

---