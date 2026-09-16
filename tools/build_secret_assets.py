#!/usr/bin/env python3
"""Rebuild credited vector props. Requires CairoSVG, only at asset-build time."""
from pathlib import Path
import cairosvg
ROOT=Path(__file__).resolve().parents[1]
cairosvg.svg2png(url=str(ROOT/'tools/sourceboards/ataturk-signature.svg'),write_to=str(ROOT/'assets/ataturk-signature.png'),output_width=256,output_height=92)
cairosvg.svg2png(url=str(ROOT/'tools/sourceboards/flag.svg'),write_to=str(ROOT/'assets/hidden-flag.png'),output_width=120,output_height=80)
