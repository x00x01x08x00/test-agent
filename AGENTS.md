# Repository review guidance

For C and Linux code, prioritize:

- kernel/userspace API semantics
- file-descriptor and other resource lifetime
- memory safety and undefined behavior
- error handling
- event-loop correctness
- tests that exercise observable behavior

Do not assume that successful compilation implies correct runtime behavior.
