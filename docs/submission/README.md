# Final submission package (Team 4, due 2026-11-29)

Every member uploads ONE zip: `TermProject_<Surname>_<Name>_<StudentID>.zip`, containing one folder with the same name:

| File | Spec item | Where it comes from |
| --- | --- | --- |
| `CSE201_ATM_Final_Report_Team4.pdf` | Submission 1 | Copy `docs/submission/CSE201_ATM_Final_Report_Team4.pdf` as is (104 pages, ends with "8. Appendix: Final Source Code") |
| `src/*.h`, `src/*.cpp` | Submission 2 | `src/` on `main` |
| `Makefile`, `CMakeLists.txt`, `README.md`, `data/initial_conditions.txt` | build support | repo root |
| `Contribution_Table_<Name>_<StudentID>.pdf` | Submission 3 (personal) | Make your own, using `docs/submission/Contribution_Table_Alan_202511195.pdf` as the template: all 4 members with IDs, score 5 for everyone including yourself, "Submitted by" = you |

Leave out: `.git/`, `build/`, `atm`, `CSE201_01_all_materials (3).zip`.

Student IDs: Rabia 202511204, Altair 202511196, Amina 202511197, Alan 202511195.

Check: unzip it, run `make` and `./atm data/initial_conditions.txt`, then type `/` and you should see the snapshot.

`docs/reference/` has the course spec, the sample demo test routine, the course sample input file and the course `file_input.cpp` example.
