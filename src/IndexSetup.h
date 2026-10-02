#ifndef SHOEBOX_INDEX_SETUP_H
#define SHOEBOX_INDEX_SETUP_H

// Ensure every attribute Shoebox queries on exists as a live BFS index on all
// writable, query-capable volumes. Idempotent; safe to call on every launch.
void SetupShoeboxIndices();

#endif
