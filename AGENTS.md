# Agents

**Start here.** This file is the operational contract for AI coding agents in this repository. Humans should read **`README.md`** for project overview and getting started.

**OpenSpec narrative and slash commands:** **`OPENSPEC_FLOW.md`**

---

## Operating model and preflight

- **Role:** Act as a focused assistant making surgical edits and additions. Keep changes minimal and precise; answer questions clearly when asked. When unsure, present options with pros/cons. Edit source code files without stopping to ask for confirmation unless it is clear the human was not asking for source edits.
- **How to use this file:** Treat it as the table of contents. Use the guide index below to locate authoritative context for the task. Default to slightly more context if unsure.
- **Preflight checklist** (before any edit, significant decision, or substantive answer):
  - Read this file and any linked guides that might apply. If it is unclear, make a note in your debrief and read them just in case.
  - Identify what applies; do not proceed until preflight is complete.
  - Every debrief must include a list of guides read, and which were or were not useful and why.

---

## Guide index (`ai/`)

| Guide | Consult when |
|-------|----------------|
| **`README.md`** | Scoping tasks; validating high-level goals |
| **`ai/code_structure.md`** | Adding/moving files; creating/updating modules; choosing folders; shared helpers; header vs cpp |
| **`ai/functional_programming.md`** | Defining/updating state; update loops; ISR boundaries; mappers/query helpers |
| **`ai/hardware.md`**, **`ai/library_reference/`** | Pins/roles/clocks; I2S buffer/timing; driver calls; ADC/attenuation |
| **`ai/modules/`** | Designing/refactoring a module; aligning implementation to intended behavior |

**Navigation:** Start from this file. Complete preflight and consult relevant guides before edits. Prefer comprehensive sources (e.g. project summaries). Skip clearly irrelevant files only after preflight confirms they are not needed.

---

## Preferences

- Keep outputs **concise, explicit, and technically precise**.
- Default to industry best practices (naming, error handling, modularity).
- Where assumptions are necessary, clearly mark them.
- Always ground responses in available repo context before guessing.
- Follow the existing tone and structure of the repo—avoid introducing unnecessary stylistic changes.

---

## Documentation rules (avoid code drift)

- Do not reference specific source file paths, classes, or function names in `ai/` docs.
- Prefer generic examples that convey patterns rather than concrete project identifiers.
- If you must include code snippets, make them self-contained and decoupled from current filenames/symbols.

---

## Priority of sources

When sources conflict, follow the higher item:

1. **OpenSpec living specs** (`openspec/specs/`) for agreed behavioral intent (see OpenSpec workflow below)
2. **This file** and linked guides in **`ai/`** for implementation patterns
3. **Project configuration** (`src/config/constants.h`)
4. **Current codebase patterns**
5. **General/industry best practices**

**Conflict handling:** If existing code diverges from the guides, treat it as code drift. Prefer aligning code to the guides. If a guide seems outdated or insufficient, pause and propose a minimal guide update; implement only after the guide is updated.

**Intentional divergence:** To change established patterns, update the relevant guide(s) first. Reference the guide change in the edit/PR summary (e.g. “Following updated functional_programming §X: …”).

---

## OpenSpec-first workflow

**OpenSpec is the control plane for intent** in this repository: goals, behavioral requirements, and how work gets approved are expressed under **`openspec/`** as structured artifacts. Casual chat, ad hoc markdown, and implementation notes are context; when they disagree with **reconciled** living specs under **`openspec/specs/`**, **treat the specs as authoritative** until a human updates them through a proper change and archive.

Work generally moves in this order:

1. **Shape** — Intent and the human review surface live in **`openspec/changes/<name>/`** (for example `proposal.md`, `design.md`, delta specs, `tasks.md`). Entry points: **`/osf-explore`** (read-only thinking) and **`/osf-propose`** (create or refine the change). This stage **does not** reconcile living specs or land on the default branch by itself.
2. **Apply** — **`/osf-apply-changes`** spawns **`osf-apply-start`** workers that implement approved **`tasks.md`** (including edits outside **`openspec/`** when tasks require them). Living specs stay read-only here; course corrections go through **`osf-apply-abort`** and revised **`/osf-propose`**, not silent edits to locked intent.
3. **Reconcile** — **`osf-apply-finish`** runs archive so deltas merge into **`openspec/specs/`**, then merges the execution branch into the **default branch** and pushes when that is the agreed completion. After archive, **`openspec/specs/`** is the durable source of behavioral truth for merged work.

**Apply-complete vs merge-complete:** merge and archive success is **merge-complete**. **Apply-complete** additionally requires every non-deferred **`tasks.md`** row—especially build/release and live-environment acceptance—to have class-appropriate evidence (or an explicit same-message override). If ops work is missing, workers **abort** rather than check boxes and finish; finish fails closed when verification notes lack ops proof. See **`OPENSPEC_FLOW.md`** → “Apply-complete vs merge-complete.”

Higher-level narrative, vocabulary, and slash-command overview: **`OPENSPEC_FLOW.md`**.

### Specs, changes, and review

- **Living specs** in **`openspec/specs/`** describe the system’s **current agreed behavior**. Prefer short Task prompts that point workers at the relevant spec files and **`openspec/changes/`** rather than re-typing requirements in chat.
- **Any change to reconciled spec content**—additions, edits, **removals**, retirements, or reorganizing spec domains—goes through **`openspec/changes/<name>/`** and the archive step. That includes work that feels like “cleanup” or “obvious,” if it would change what **`openspec/specs/`** is allowed to say. **Bypass** only when the human **explicitly** asks for a one-off direct edit (say so in chat).
- **Review bias:** Validate requirement deltas and task outcomes; do not assume humans will read every generated line. If implementation and specs disagree, fix it with a spec change and archive, not unreviewed drift.
- **OpenSpec CLI:** `npx @fission-ai/openspec@latest <command>` when `openspec` is not installed globally. **Node.js ≥ 20.19** is required for the CLI.

### `openspec/` directory discipline

- **Reading** **`openspec/specs/`** and active **`openspec/changes/`** for context is expected.
- **Do not** create, edit, move, or delete paths under **`openspec/`** outside the OpenSpec workflow. Use the OpenSpec CLI and the repo skills/agents (**`/osf-explore`**, **`/osf-propose`**, **`/osf-apply-changes`**, and the **`osf-apply-*`** Task agents) so scaffolding, sequencing, **`tasks.md`** checkoffs, and archive merges stay consistent with the tool.
- **Living specs (`openspec/specs/`)** change **only** as the documented result of **archiving** an approved change (merge into living specs via **`osf-apply-finish`** or the same outcome run explicitly by a human)—never because an agent “caught up” with hand edits.
- **During apply (`osf-apply-start`)**, routine edits inside **`openspec/changes/<name>/`** are **`tasks.md` checkbox** updates unless the human widens scope. **Do not** reconcile **`openspec/specs/`** during apply.
- **Narrow exceptions** (state in chat when you use one): the human orders a **one-off bypass**; or a skill/agent definition explicitly allows a mechanical edit (for example marking **`[x]`** in **`tasks.md`** per **`osf-apply-start`**). Those exceptions **are not** permission to rewrite **`openspec/specs/`** by hand.
- Skills **`/osf-propose`** and **`/osf-explore`** must not merge to the default branch or archive changes **unless** the human clearly asked for apply/finish in the same turn.

---

## Cursor integration and nomenclature

- **Chat skills** live under **`.cursor/skills/*/SKILL.md`** — they steer the conversational agent’s procedure.
- **Task agents** live under **`.cursor/agents/<name>.md`** — Cursor runs them via the **Task** tool with **`subagent_type`** equal to **`name`**. Opening an agent definition and replaying its steps in the parent thread **breaks isolation**; do not inline **`osf-apply-*`** workflows as a substitute for Task delegation.
- OpenSpec workflow artifacts use the **`osf-*`** prefix — skills at **`.cursor/skills/osf-*/SKILL.md`** and apply/finish/abort agents at **`.cursor/agents/osf-apply-*.md`**.

**OSF apply workers MUST use Tasks:**

- **`osf-apply-start`**, **`osf-apply-finish`**, and **`osf-apply-abort`** MUST be invoked via the **Task** tool with the matching **`subagent_type`**. Do not read **`.cursor/agents/osf-*.md`** and replay them in the parent thread.
- Follow **`.cursor/skills/osf-apply-changes/SKILL.md`**.

---

## Git branches and default branch

Unless the user explicitly asks otherwise: **commit and push on the current working branch** for day-to-day work. **`osf-apply-finish`** merges the execution branch into the repository’s **default branch** when completing a change—only do that within that finish workflow or when the user explicitly requests it.

---

## Safety and environments

- No destructive actions on machines, data, or shared infrastructure **unless** the human named the target and risk is acceptable.
- Honor project-specific allowlists, staging requirements, and secrets handling; when in doubt, stop and ask.
- If the project records **environment or target policies** in specs or docs (for example allowed hosts or data classes), **follow those** during apply; do not substitute your own assumptions.

---

## Environment variables

Use **`.env`** for local secrets (API keys for optional tools). **`.env`** should remain gitignored.

---

## Reference

| Document | Role |
|----------|------|
| **`OPENSPEC_FLOW.md`** | Narrative, vocabulary, **`OPENSPEC_FLOW_VERSION`**, capability table |
| **`README.md`** | Human-facing project overview, code layout, hardware pointers |
| **`CHANGELOG.md`** | History of OpenSpec Flow bundle changes (when upgrading the bundle) |
| **`.cursor/skills/openspec-flow-install/SKILL.md`** | Install or upgrade the OpenSpec Flow bundle |
