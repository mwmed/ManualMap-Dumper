# Note
This will only detect & dump externally allocated pages regardless of whether the page is allocated from kernel or usermode

# Tip
In order to detect manually mapped dlls injected into unused rwx pages search for the 'MZ' sig and validate the header, check if the base address is a module( to prevent dumping legit modules ) and dump it.
