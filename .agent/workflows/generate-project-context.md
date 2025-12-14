---
description: Generate consolidated project_context.md from multiple source documents
---

# Workflow: Generate Project Context

This workflow creates a unified `project_context.md` by aggregating several source documentation files.

## Steps

1. **Gather source file paths**
   - Ensure the following source files exist:
     - `/Users/gabeochoa/p/prime_pressure/docs/reference_project_context.md`
     - `/Users/gabeochoa/p/prime_pressure/docs/architecture.md`
     - `/Users/gabeochoa/p/prime_pressure/docs/reference_architecture.md`
     - `/Users/gabeochoa/p/prime_pressure/docs/GDD.md`
     - `/Users/gabeochoa/p/prime_pressure/docs/epics.md`

2. **Create a temporary aggregation script**
   ```bash
   cat <<'EOF' > /tmp/aggregate_project_context.sh
   #!/usr/bin/env bash
   set -euo pipefail
   OUTPUT="${1}"
   shift
   echo "# Project Context" > "$OUTPUT"
   for SRC in "$@"; do
     echo "\n---\n" >> "$OUTPUT"
     echo "## $(basename "$SRC" .md)" >> "$OUTPUT"
     cat "$SRC" >> "$OUTPUT"
   done
   EOF
   chmod +x /tmp/aggregate_project_context.sh
   ```
   // turbo

3. **Run the aggregation script**
   ```bash
   /tmp/aggregate_project_context.sh \
     /Users/gabeochoa/p/prime_pressure/docs/project_context.md \
     /Users/gabeochoa/p/prime_pressure/docs/reference_project_context.md \
     /Users/gabeochoa/p/prime_pressure/docs/architecture.md \
     /Users/gabeochoa/p/prime_pressure/docs/reference_architecture.md \
     /Users/gabeochoa/p/prime_pressure/docs/GDD.md \
     /Users/gabeochoa/p/prime_pressure/docs/epics.md
   ```
   // turbo

4. **Verify output**
   - Open `/Users/gabeochoa/p/prime_pressure/docs/project_context.md` and ensure the sections are correctly concatenated.

5. **Cleanup**
   - Optionally remove the temporary script:
   ```bash
   rm /tmp/aggregate_project_context.sh
   ```

---

**Usage**
Run the workflow by executing the steps in order. Steps marked with `// turbo` can be auto‑executed with the `run_command` tool when integrated into an automated pipeline.
