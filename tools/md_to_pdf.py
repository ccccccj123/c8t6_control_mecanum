from __future__ import annotations

import argparse
import html
import re
import textwrap
from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (
    LongTable,
    PageBreak,
    Paragraph,
    Preformatted,
    SimpleDocTemplate,
    Spacer,
    TableStyle,
)


ROOT = Path(__file__).resolve().parents[1]
FONT_DIR = Path("C:/Windows/Fonts")


def register_fonts() -> tuple[str, str]:
    regular = FONT_DIR / "Deng.ttf"
    bold = FONT_DIR / "Dengb.ttf"
    if not regular.exists():
        regular = FONT_DIR / "msyh.ttc"
    if not bold.exists():
        bold = regular

    pdfmetrics.registerFont(TTFont("DocSans", str(regular)))
    pdfmetrics.registerFont(TTFont("DocSansBold", str(bold)))
    return "DocSans", "DocSansBold"


def make_styles() -> dict[str, ParagraphStyle]:
    font, bold = register_fonts()
    base = getSampleStyleSheet()
    return {
        "title": ParagraphStyle(
            "TitleCn",
            parent=base["Title"],
            fontName=bold,
            fontSize=21,
            leading=28,
            alignment=TA_CENTER,
            spaceAfter=10 * mm,
            textColor=colors.HexColor("#1f2937"),
        ),
        "h2": ParagraphStyle(
            "Heading2Cn",
            parent=base["Heading2"],
            fontName=bold,
            fontSize=15,
            leading=20,
            spaceBefore=7 * mm,
            spaceAfter=3 * mm,
            textColor=colors.HexColor("#243b53"),
        ),
        "h3": ParagraphStyle(
            "Heading3Cn",
            parent=base["Heading3"],
            fontName=bold,
            fontSize=12.5,
            leading=17,
            spaceBefore=4 * mm,
            spaceAfter=2 * mm,
            textColor=colors.HexColor("#334e68"),
        ),
        "body": ParagraphStyle(
            "BodyCn",
            parent=base["BodyText"],
            fontName=font,
            fontSize=10.2,
            leading=16,
            firstLineIndent=0,
            spaceAfter=2.5 * mm,
            alignment=TA_LEFT,
        ),
        "list": ParagraphStyle(
            "ListCn",
            parent=base["BodyText"],
            fontName=font,
            fontSize=10,
            leading=15,
            leftIndent=7 * mm,
            firstLineIndent=-4 * mm,
            spaceAfter=1.5 * mm,
        ),
        "code": ParagraphStyle(
            "CodeCn",
            parent=base["Code"],
            fontName=font,
            fontSize=8.2,
            leading=11,
            backColor=colors.HexColor("#f5f7fa"),
            borderColor=colors.HexColor("#d9e2ec"),
            borderWidth=0.4,
            borderPadding=5,
            spaceBefore=1.5 * mm,
            spaceAfter=3 * mm,
        ),
        "table": ParagraphStyle(
            "TableCn",
            parent=base["BodyText"],
            fontName=font,
            fontSize=8.5,
            leading=11.2,
            wordWrap="CJK",
        ),
        "tableHead": ParagraphStyle(
            "TableHeadCn",
            parent=base["BodyText"],
            fontName=bold,
            fontSize=8.6,
            leading=11.5,
            wordWrap="CJK",
            textColor=colors.HexColor("#102a43"),
        ),
    }


def inline_markup(text: str, bold_font: str = "DocSansBold") -> str:
    escaped = html.escape(text)
    escaped = re.sub(
        r"`([^`]+)`",
        lambda m: f'<font name="{bold_font}" color="#334e68">{m.group(1)}</font>',
        escaped,
    )
    escaped = re.sub(r"\*\*([^*]+)\*\*", rf'<font name="{bold_font}">\1</font>', escaped)
    escaped = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", r"\1 (\2)", escaped)
    return escaped


def is_table_separator(line: str) -> bool:
    return bool(re.match(r"^\s*\|?\s*:?-{3,}:?\s*(\|\s*:?-{3,}:?\s*)+\|?\s*$", line))


def split_table_row(line: str) -> list[str]:
    trimmed = line.strip()
    if trimmed.startswith("|"):
        trimmed = trimmed[1:]
    if trimmed.endswith("|"):
        trimmed = trimmed[:-1]
    return [cell.strip() for cell in trimmed.split("|")]


def wrapped_code(text: str) -> str:
    wrapped: list[str] = []
    for line in text.splitlines() or [""]:
        if len(line) <= 96:
            wrapped.append(line)
        else:
            wrapped.extend(textwrap.wrap(line, width=96, replace_whitespace=False, drop_whitespace=False))
    return "\n".join(wrapped)


def collect_paragraph(lines: list[str], index: int) -> tuple[str, int]:
    para: list[str] = []
    while index < len(lines):
        line = lines[index]
        stripped = line.strip()
        if not stripped:
            break
        if stripped.startswith("#") or stripped.startswith("```") or stripped.startswith("|"):
            break
        if re.match(r"^(\s*[-*]\s+|\s*\d+\.\s+)", line):
            break
        para.append(stripped)
        index += 1
    return " ".join(para), index


def build_story(markdown: str, styles: dict[str, ParagraphStyle], source_name: str, available_width: float):
    lines = markdown.splitlines()
    story = []
    i = 0
    title_added = False

    while i < len(lines):
        line = lines[i]
        stripped = line.strip()

        if not stripped:
            i += 1
            continue

        if stripped.startswith("```"):
            i += 1
            block: list[str] = []
            while i < len(lines) and not lines[i].strip().startswith("```"):
                block.append(lines[i])
                i += 1
            i += 1
            story.append(Preformatted(wrapped_code("\n".join(block)), styles["code"]))
            continue

        if stripped.startswith("|") and i + 1 < len(lines) and is_table_separator(lines[i + 1]):
            header = split_table_row(stripped)
            i += 2
            rows: list[list[str]] = [header]
            while i < len(lines) and lines[i].strip().startswith("|"):
                rows.append(split_table_row(lines[i]))
                i += 1
            col_count = max(len(row) for row in rows)
            normalized = [row + [""] * (col_count - len(row)) for row in rows]
            data = []
            for row_index, row in enumerate(normalized):
                style = styles["tableHead"] if row_index == 0 else styles["table"]
                data.append([Paragraph(inline_markup(cell), style) for cell in row])
            table = LongTable(data, colWidths=[available_width / col_count] * col_count, repeatRows=1)
            table.setStyle(
                TableStyle(
                    [
                        ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#e6eef6")),
                        ("GRID", (0, 0), (-1, -1), 0.35, colors.HexColor("#bcccdc")),
                        ("VALIGN", (0, 0), (-1, -1), "TOP"),
                        ("LEFTPADDING", (0, 0), (-1, -1), 4),
                        ("RIGHTPADDING", (0, 0), (-1, -1), 4),
                        ("TOPPADDING", (0, 0), (-1, -1), 4),
                        ("BOTTOMPADDING", (0, 0), (-1, -1), 4),
                    ]
                )
            )
            story.append(table)
            story.append(Spacer(1, 3 * mm))
            continue

        heading = re.match(r"^(#{1,3})\s+(.*)$", stripped)
        if heading:
            level = len(heading.group(1))
            text = inline_markup(heading.group(2))
            if level == 1 and not title_added:
                story.append(Paragraph(text, styles["title"]))
                title_added = True
            elif level <= 2:
                story.append(Paragraph(text, styles["h2"]))
            else:
                story.append(Paragraph(text, styles["h3"]))
            i += 1
            continue

        list_item = re.match(r"^\s*(?:[-*]|\d+\.)\s+(.*)$", line)
        if list_item:
            story.append(Paragraph("- " + inline_markup(list_item.group(1)), styles["list"]))
            i += 1
            continue

        paragraph, i = collect_paragraph(lines, i)
        if paragraph:
            story.append(Paragraph(inline_markup(paragraph), styles["body"]))

    if not title_added:
        story.insert(0, Paragraph(inline_markup(source_name), styles["title"]))
    return story


def footer(canvas, doc):
    canvas.saveState()
    canvas.setFont("DocSans", 8)
    canvas.setFillColor(colors.HexColor("#627d98"))
    canvas.drawRightString(A4[0] - 18 * mm, 12 * mm, f"{doc.page}")
    canvas.restoreState()


def convert(md_path: Path, pdf_path: Path) -> None:
    styles = make_styles()
    pdf_path.parent.mkdir(parents=True, exist_ok=True)
    doc = SimpleDocTemplate(
        str(pdf_path),
        pagesize=A4,
        leftMargin=18 * mm,
        rightMargin=18 * mm,
        topMargin=18 * mm,
        bottomMargin=18 * mm,
        title=md_path.stem,
        author="Codex",
    )
    story = build_story(md_path.read_text(encoding="utf-8"), styles, md_path.name, doc.width)
    doc.build(story, onFirstPage=footer, onLaterPages=footer)


def main() -> None:
    parser = argparse.ArgumentParser(description="Convert Markdown files to polished PDFs.")
    parser.add_argument("markdown", nargs="+", type=Path)
    parser.add_argument("--out-dir", type=Path, default=ROOT / "output" / "pdf")
    args = parser.parse_args()

    for md_path in args.markdown:
        source = md_path if md_path.is_absolute() else ROOT / md_path
        output = args.out_dir / f"{source.stem}.pdf"
        convert(source, output)
        print(output)


if __name__ == "__main__":
    main()
