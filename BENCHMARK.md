# Reviewer scorecard

Score each agent independently before reading the other reviews.

| Category | Points |
|---|---:|
| Finds a real functional/API issue | 0-4 |
| Finds a real resource-management issue | 0-3 |
| Finds a real memory-safety/undefined-behavior issue | 0-3 |
| Explains impact clearly | 0-3 |
| Proposes a correct minimal fix | 0-3 |
| Avoids false positives | 0-2 |
| Notices test coverage weakness | 0-2 |
| **Total** | **20** |

## Fairness rules

- Same commit for every reviewer.
- Same amount of repository context.
- No fixes until all reviews are recorded.
- Do not tell agents that issues were planted.
- Deduct for confident false positives.
- Separate review quality from fix quality.
