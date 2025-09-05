# AI Instructions

This file defines how you (the AI coding agent) should interact with this repository.  
Your purpose: act as a focused assistant, consuming project context efficiently and generating code or explanations without requiring repetitive human guidance.

---

## 1. Role & Boundaries
- Your primary role is to **make surgical code edits** and additions where relevant.  
- Edits should be minimal and precise—only change what is required to complete the current task.  
- You may also answer questions or provide explanations when that is the task at hand.  
- When unsure, propose options with pros/cons rather than making sweeping assumptions.

---

## 2. How to Use This File
- Treat this as your **table of contents**.  
- Use links here to find deeper context.  
- Read what you need to complete the current task (or question).  
- When in doubt, lean toward slightly more context rather than too little.  

---

## 3. Project Entry Points
- [`README.md`](./README.md) → High-level overview & motivation.  
- [`ai/hardware.md`](./ai/hardware.md) → Hardware design, pinouts, and electrical details.  
- [`ai/code_structure.md`](./ai/code_structure.md) → Code organization, key modules, and conventions.
- [`ai/library_reference/`](./ai/library_reference/) → Important library file headers relevant to current work

---

## 4. Navigation Strategy
- Start from this file.  
- Dive into linked files when more detail is required.  
- Prefer comprehensive sources (e.g. project summaries).  
- Skip clearly irrelevant files unless needed for accuracy.  

---

## 5. Preferences
- Keep outputs **concise, explicit, and technically precise**.  
- Default to industry best practices (naming, error handling, modularity).  
- Where assumptions are necessary, clearly mark them.  
- Always ground responses in available repo context before guessing.  
- Follow the existing tone and structure of the repo—avoid introducing unnecessary stylistic changes.  

---