---

# 📊 Project Phase 3 — Data Systems (Spring 2025)

**Due Date:** 11:59 PM, Friday, 18th April 2025  

---

## 📌 Instructions

- This is a **team project** — you’ll work in the teams assigned previously.
- Build upon your code-base from earlier phases.
- All commits must be pushed to the **`master` branch** of your GitHub Classroom repository.
  - You may work on separate branches but **merge them into master before the due date**.
- No personal queries will be entertained. Please post all questions on **Moodle**.
- If any commit is made past the deadline, fill out a **Late Day Form** on the day of submission and mention the correct date of your last commit.
- Not following these instructions will result in a penalty.

---

## ⚙️ Commands & Functionality

This phase focuses on implementing **Indexing structures** and integrating them into SQL query operations.

- Choose any suitable indexing structure but provide a justification for your choice in the final report.
- You’ll be graded on both **correctness** and **efficiency**.
- Assume test cases will use large tables **exceeding main memory capacity**.
  - Maximum memory allowance: **10 blocks at any time**.
- **Submissions without indexing** will receive **zero marks**.
- Supported operators for conditions:
  ```
  <, <=, >, >=, ==, !=
  ```
- Conditions are in the format:
  ```
  col_name bin_op value
  ```

---

## 📖 Command Specifications

### 🔍 2.1 SEARCH

**Syntax:**
```sql
res_table <- SEARCH FROM table_name WHERE condition
```

**Example:**
**Table:** `Students`

| Stud_ID | phy | chem | math |
|:--------|:-----|:------|:------|
| 1 | 36 | 41 | 29 |
| 2 | 49 | 49 | 20 |
| 3 | 37 | 43 | 49 |
| 4 | 47 | 44 | 50 |
| 5 | 17 | 21 | 9 |
| 6 | 10 | 27 | 26 |
| 7 | 40 | 6 | 9 |

**Query:**
```sql
exampleResult <- SEARCH FROM Students WHERE math >= 30
```

**Result:**
| Stud_ID | phy | chem | math |
|:--------|:-----|:------|:------|
| 3 | 37 | 43 | 49 |
| 4 | 47 | 44 | 50 |

**Notes:**
- `SEARCH` works like the existing `SELECT` but uses **Indexing** for optimization.
- Do **not modify** the existing `SELECT` implementation in `SimpleRA`.
- Implement a **new SEARCH command** leveraging Indexing.

---

### ➕ 2.2 INSERT

**Syntax:**
```sql
INSERT INTO table_name ( col1 = val1, col2 = val2, col3 = val3, … )
```

**Details:**
- Modifies the table in-place.
- Any unspecified columns should default to `0`.
- At least one `col_name = value` pair will be provided.

---

### ✏️ 2.3 UPDATE

**Syntax:**
```sql
UPDATE table_name WHERE condition SET col_name = value
```

**Details:**
- Modifies the table in-place.
- Updates every record satisfying the condition.
- No operation is performed if no records match.

---

### ❌ 2.4 DELETE

**Syntax:**
```sql
DELETE FROM table_name WHERE condition
```

**Details:**
- Modifies the table in-place.
- Deletes every record satisfying the condition.
- No operation is performed if no records match.
