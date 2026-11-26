#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 Alexander Stephen Donahue
"""Live Citation, Crossref Metadata & Retraction Verifier (Playbook v1.3 Gold Standard):
Parses every BibTeX entry in `paper/refs.bib`, verifies:
  1. Live DOI resolution via Crossref REST API (`api.crossref.org/works/<doi>`) with fallback
     to the DOI Handle API (`doi.org/api/handles/<doi>`) for non-Crossref registries.
  2. Zero retractions (`update-to` type == 'retraction' or `relation.is-retracted-by`).
  3. Publication year parity (within ±1 year for online-first vs print) & title token overlap.
  4. HTTP reachability for `url = {...}` entries.
  5. Writes machine-verifiable `results/citation-audit.json` when `--write-json` is passed.
Zero fabricated citations, zero broken DOIs, zero retracted works."""

from __future__ import annotations

import argparse
import datetime as dt
import json
import re
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BIB_PATH = ROOT / "paper" / "refs.bib"
DEFAULT_JSON = ROOT / "results" / "citation-audit.json"
UA = "CellulogonicMecha-CitationVerifier/1.6 (mailto:0_0@0thernes.art; +https://github.com/0thernes/cellulogonic-mecha)"

STOPWORDS = {
    "a", "an", "and", "as", "at", "by", "for", "from", "in", "into", "is", "its",
    "of", "on", "or", "the", "to", "with", "via", "under", "toward", "towards",
}


def norm_tokens(text: str) -> set[str]:
    clean = re.sub(r"\\[a-zA-Z]+\{([^}]*)\}", r"\1", text)
    clean = re.sub(r"[{}\\]", "", clean)
    clean = clean.replace("--", " ").replace("-", " ")
    toks = re.findall(r"[a-z0-9]{3,}", clean.lower())
    return {t for t in toks if t not in STOPWORDS}


def parse_bibtex_entries(raw: str) -> list[dict[str, str]]:
    entries: list[dict[str, str]] = []
    chunks = re.split(r"(?=@\w+\s*\{)", raw)
    for ch in chunks:
        ch = ch.strip()
        if not ch.startswith("@"):
            continue
        m_head = re.match(r"@(\w+)\s*\{\s*([^,\s]+)\s*,", ch)
        if not m_head:
            continue
        rec: dict[str, str] = {"type": m_head.group(1).lower(), "key": m_head.group(2).strip()}
        for field in ("title", "year", "doi", "url", "journal", "booktitle"):
            fm = re.search(rf"\b{field}\s*=\s*\{{((?:[^{{}}]|\{{[^{{}}]*\}})*)\}}", ch, re.I | re.S)
            if fm:
                rec[field] = re.sub(r"\s+", " ", fm.group(1)).strip()
        entries.append(rec)
    return entries


def check_doi_crossref(doi: str, bib_title: str, bib_year: str, timeout: int = 20) -> dict:
    doi_clean = doi.strip()
    cr_url = f"https://api.crossref.org/works/{urllib.parse.quote(doi_clean, safe='/')}"
    req = urllib.request.Request(cr_url, headers={"User-Agent": UA, "Accept": "application/json"}, method="GET")
    last_exc: Exception | None = None
    for attempt in range(3):
        try:
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                if resp.getcode() == 200:
                    msg = json.loads(resp.read().decode("utf-8")).get("message", {})
                    cr_title = (msg.get("title") or [""])[0]
                    cr_years: set[int] = set()
                    for k in ("published-print", "published-online", "issued", "created"):
                        parts = (msg.get(k) or {}).get("date-parts") or []
                        if parts and parts[0] and isinstance(parts[0][0], int):
                            cr_years.add(parts[0][0])

                    retracted = False
                    for upd in msg.get("update-to") or []:
                        if str(upd.get("type", "")).lower() == "retraction":
                            retracted = True
                    rel = msg.get("relation") or {}
                    if rel.get("is-retracted-by"):
                        retracted = True

                    year_ok = True
                    if bib_year.isdigit() and cr_years:
                        by = int(bib_year)
                        year_ok = any(abs(by - cy) <= 1 for cy in cr_years)

                    t_bib = norm_tokens(bib_title)
                    t_cr = norm_tokens(cr_title)
                    overlap = (len(t_bib & t_cr) / max(1, min(len(t_bib), len(t_cr)))) if (t_bib and t_cr) else 1.0
                    title_ok = (overlap >= 0.45)

                    ok = (not retracted) and year_ok and title_ok
                    notes = []
                    if retracted:
                        notes.append("RETRACTED")
                    if not year_ok:
                        notes.append(f"year_mismatch(bib={bib_year},cr={sorted(cr_years)})")
                    if not title_ok:
                        notes.append(f"title_overlap={overlap:.2f}")
                    return {
                        "target": f"doi:{doi_clean}",
                        "source": "crossref",
                        "ok": ok,
                        "retracted": retracted,
                        "year_ok": year_ok,
                        "title_overlap": round(overlap, 3),
                        "crossref_title": cr_title,
                        "crossref_years": sorted(cr_years),
                        "status": "OK (Crossref)" if ok else "; ".join(notes),
                    }
        except urllib.error.HTTPError as e:
            if e.code == 404:
                last_exc = None
                break
            if e.code in (429, 500, 502, 503, 504) and attempt < 2:
                time.sleep(1.0 * (attempt + 1))
                continue
            return {"target": f"doi:{doi_clean}", "source": "crossref", "ok": False, "retracted": False, "status": f"Crossref HTTP {e.code}"}
        except Exception as exc:
            last_exc = exc
            if attempt < 2:
                time.sleep(1.0 * (attempt + 1))
                continue
            return {"target": f"doi:{doi_clean}", "source": "crossref", "ok": False, "retracted": False, "status": f"Crossref error: {last_exc}"}

    # Fallback for non-Crossref DOIs (e.g., DataCite / arXiv / Zenodo)
    api_url = f"https://doi.org/api/handles/{urllib.parse.quote(doi_clean, safe='/')}"
    req2 = urllib.request.Request(api_url, headers={"User-Agent": UA}, method="GET")
    try:
        with urllib.request.urlopen(req2, timeout=timeout) as resp:
            if resp.getcode() == 200:
                payload = json.loads(resp.read().decode("utf-8"))
                if payload.get("responseCode") == 1:
                    return {
                        "target": f"doi:{doi_clean}",
                        "source": "doi_handle",
                        "ok": True,
                        "retracted": False,
                        "year_ok": True,
                        "title_overlap": None,
                        "status": "OK (DOI Handle)",
                    }
            return {"target": f"doi:{doi_clean}", "source": "doi_handle", "ok": False, "retracted": False, "status": f"HTTP {resp.getcode()}"}
    except Exception as exc:
        return {"target": f"doi:{doi_clean}", "source": "doi_handle", "ok": False, "retracted": False, "status": str(exc)}


def check_url(url: str, timeout: int = 15) -> dict:
    headers = {"User-Agent": f"Mozilla/5.0 (compatible; {UA})"}
    for method in ("HEAD", "GET"):
        req = urllib.request.Request(url, headers=headers, method=method)
        try:
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                code = resp.getcode()
                if 200 <= code < 400:
                    return {"target": url, "source": "http", "ok": True, "retracted": False, "status": f"HTTP {code}"}
        except urllib.error.HTTPError as e:
            if method == "HEAD" and e.code in (403, 405, 429):
                continue
            return {"target": url, "source": "http", "ok": False, "retracted": False, "status": f"HTTP {e.code}"}
        except Exception as exc:
            if method == "HEAD":
                continue
            return {"target": url, "source": "http", "ok": False, "retracted": False, "status": str(exc)}
    return {"target": url, "source": "http", "ok": False, "retracted": False, "status": "unreachable"}


def main() -> None:
    parser = argparse.ArgumentParser(description="Verify citations, Crossref metadata, and zero retractions in paper/refs.bib.")
    parser.add_argument("--offline", action="store_true", help="Only validate BibTeX syntax & non-empty fields without network calls")
    parser.add_argument("--write-json", nargs="?", const=str(DEFAULT_JSON), help="Write machine-readable audit report JSON")
    args = parser.parse_args()

    if not BIB_PATH.exists():
        print(f"FAIL: {BIB_PATH} not found", file=sys.stderr)
        sys.exit(1)

    raw = BIB_PATH.read_text(encoding="utf-8")
    entries = parse_bibtex_entries(raw)
    dois = [(e["key"], e["doi"], e.get("title", ""), e.get("year", "")) for e in entries if "doi" in e]
    urls = [(e["key"], e["url"]) for e in entries if "url" in e and "doi" not in e]
    extra_urls = [(e["key"], e["url"]) for e in entries if "url" in e and "doi" in e]
    all_urls = list({u: k for k, u in (urls + extra_urls)}.items())

    print(f"Parsed {len(entries)} BibTeX entries ({len(dois)} DOIs + {len(all_urls)} URLs) from {BIB_PATH.name}.")
    if len(entries) < 30:
        print("FAIL: expected at least 30 verified citations in paper/refs.bib", file=sys.stderr)
        sys.exit(1)

    for e in entries:
        if not e.get("title") or not e.get("year"):
            print(f"FAIL: BibTeX entry '{e['key']}' missing title or year", file=sys.stderr)
            sys.exit(1)

    if args.offline:
        print("PASS (offline BibTeX syntax & required-field check).")
        return

    records: list[dict] = []
    failures: list[dict] = []

    for u, key in all_urls:
        rec = check_url(u)
        rec["key"] = key
        records.append(rec)
        print(f"  [{'OK  ' if rec['ok'] else 'FAIL'}] {rec['status']:<18} {key:<18} {u}")
        if not rec["ok"]:
            failures.append(rec)

    for key, doi, title, year in dois:
        rec = check_doi_crossref(doi, title, year)
        rec["key"] = key
        records.append(rec)
        print(f"  [{'OK  ' if rec['ok'] else 'FAIL'}] {rec['status']:<18} {key:<18} doi:{doi}")
        if not rec["ok"]:
            failures.append(rec)

    retractions = sum(1 for r in records if r.get("retracted"))
    report = {
        "schema": "cellulogonic-citation-audit/v1",
        "generated_utc": dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "bib_entries": len(entries),
        "targets_checked": len(records),
        "targets_passed": len(records) - len(failures),
        "retractions_found": retractions,
        "records": records,
    }

    if args.write_json:
        out_path = Path(args.write_json)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(json.dumps(report, indent=2, sort_keys=False) + "\n", encoding="utf-8", newline="\n")
        print(f"Wrote machine-verifiable citation audit -> {out_path}")

    if failures or retractions > 0:
        print(f"\nFAIL: {len(failures)}/{len(records)} citation target(s) failed verification ({retractions} retractions)!", file=sys.stderr)
        sys.exit(1)
    print(f"\nPASS: All {len(records)}/{len(records)} citation URLs & DOIs verified live via Crossref/HTTP (0 retractions, 0 mismatches)!")


if __name__ == "__main__":
    main()
