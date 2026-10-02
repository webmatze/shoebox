# Shoebox

A BFS-native photo organizer for Haiku.

Every piece of metadata — ratings, tags, albums, captions — lives as an
attribute on the image file itself. Smart folders, albums, and tag views are
live BFS queries that update as attributes change. There is no sidecar
database, no index to rebuild, no project file. Move an image and its tags
move with it; delete it and they're gone. The filesystem *is* the database.

## Features

- Three-pane layout: smart folders / thumbnail grid / metadata inspector.
- **Live smart folders** — "All Photos", "Rated 4+", "Flagged", "This Week" —
  driven by `BQuery` with `B_LIVE_QUERY` so changes appear without a refresh.
- **Dynamic Albums and Tags** — the sidebar discovers album and tag values by
  scanning `Photo:Album` and `Photo:Tags` across every query-capable volume;
  new ones appear as soon as you type them into the inspector.
- **Thumbnail cache** stored as a `Shoebox:Thumb256` BFS attribute, so
  thumbnails travel with the file.
- **Import / Rescan** command that walks a folder and re-indexes images
  (fixes pre-existing files that were saved before the BFS indices existed).
- **Image menu** with rate (0–5), flag (pick/none/reject), open, and reveal
  in Tracker.
- Double-click a thumbnail to open the image in the default handler.

## Attributes

| Attribute | Type | Purpose |
|---|---|---|
| `Photo:Rating` | `B_INT32_TYPE` | 0..5 |
| `Photo:Flag` | `B_INT32_TYPE` | -1 reject / 0 none / +1 pick |
| `Photo:Tags` | `B_STRING_TYPE` | comma-separated |
| `Photo:Album` | `B_STRING_TYPE` | album name |
| `Photo:Caption` | `B_STRING_TYPE` | free text |
| `Shoebox:Thumb256` | `B_RAW_TYPE` | cached PNG thumbnail |
| `Media:CaptureTime` | `B_INT32_TYPE` | epoch seconds, from EXIF (TODO) |
| `Media:Camera`, `Media:Lens`, … | various | reserved for EXIF ingest |

BFS indices for the `Photo:*` and `Media:CaptureTime` attributes are created
on first launch via `fs_create_index`.

## Build

Requires Haiku with the standard devkit (gcc, make, Makefile-Engine). Just:

    make

Produces a `Shoebox` binary in the project root. Links against `libbe`,
`libtracker`, and `libtranslation` — no external dependencies.

## Running

Launch from Terminal (so stderr diagnostics show):

    ./Shoebox

Or double-click the binary from Tracker. First launch creates BFS indices
on all writable, query-capable volumes.

## Quick walkthrough

1. **File → Import folder…** → pick a folder of images. Shoebox re-writes
   each `BEOS:TYPE` so BFS inserts the files into its indices. Needed only
   for files that existed before the indices did.
2. Click **All Photos** in the sidebar — thumbnails appear.
3. Click a thumbnail → the inspector on the right shows its metadata.
4. Set a rating (type `5`, hit Enter), or use **Image → Rate ▸ 5 stars**.
5. Click **Rated 4+** — the photo is already there, no refresh.
6. Type "Vacation" into the Album field → a Vacation entry appears under
   **Albums** in the sidebar. Click it to filter.

## Architecture

Each module does one thing:

- `App`, `MainWindow` — application + three-pane window with menu bar.
- `SidebarView` — smart folders and dynamic albums/tags trees; emits
  predicate strings via `QueryItem`.
- `QueryModel` (BHandler) — wraps a `BQuery`, drains initial results,
  listens for `B_QUERY_UPDATE` live updates, publishes ref lists to the grid.
- `PhotoGridView` / `GridContent` — scrollable thumbnail grid with
  selection, click-to-select, double-click to open.
- `MetadataPanel` — reads/writes `Photo:*` attributes on the selected file.
- `TagModel` — scans distinct `Photo:Album` and `Photo:Tags` values across
  all query-capable volumes.
- `ThumbnailCache` — decodes via Translation Kit, scales in an offscreen
  `BView`, re-encodes as PNG, caches in `Shoebox:Thumb256`.
- `IndexSetup` — ensures `BEOS:TYPE` and `Photo:*`/`Media:CaptureTime`
  indices exist with the right types.
- `Importer` — walks a folder, rewrites `BEOS:TYPE` to force index insert.
- `AttributeNames.h`, `Messages.h` — the two header files everything else
  pivots around.

## Known limitations

- **Thumbnail decode is synchronous** on the window thread. Large libraries
  will stutter on first display.
- **Tag queries are substring matches** (`Photo:Tags=="*hiking*"`) and can
  false-positive: a tag "mount" also matches "mountains". Fix: switch to
  one-attribute-per-tag or sentinel-delimited values.
- **No EXIF ingest yet**, so `Media:CaptureTime` is always empty and
  "This Week" won't find anything until ingest lands.
- **No predicate escaping** of album/tag names.
- **No undo**, no multi-select, no delete.

## Status

Early. Builds, runs, usable on small libraries. Not packaged as a `.hpkg`.

## License

TBD.
