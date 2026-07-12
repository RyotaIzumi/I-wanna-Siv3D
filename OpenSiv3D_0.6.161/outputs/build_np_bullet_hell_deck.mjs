import fs from "node:fs/promises";
import { Presentation, PresentationFile } from "@oai/artifact-tool";

const OUT = "C:/Users/tomat/source/repos/I wanna Siv3D/OpenSiv3D_0.6.161/outputs/np_complete_bullet_hell_research.pptx";
const QA = "C:/Users/tomat/source/repos/I wanna Siv3D/OpenSiv3D_0.6.161/outputs/np_complete_bullet_hell_preview.webp";

const W = 1280;
const H = 720;
const C = {
  ink: "#111111",
  muted: "#555A61",
  faint: "#EDEDED",
  rule: "#B8BCC4",
  blue: "#3D8DFF",
  blueLite: "#D0EDFA",
  cyan: "#6DCBF4",
  green: "#34A853",
  red: "#EA4335",
  amber: "#F6B73C",
  white: "#FFFFFF",
};

function addText(slide, text, x, y, w, h, opts = {}) {
  const shape = slide.shapes.add({
    geometry: "textbox",
    position: { left: x, top: y, width: w, height: h },
    fill: "none",
    line: { style: "solid", fill: "none", width: 0 },
  });
  shape.text = text;
  shape.text.style = {
    fontSize: opts.size ?? 22,
    bold: opts.bold ?? false,
    color: opts.color ?? C.ink,
    alignment: opts.align ?? "left",
  };
  return shape;
}

function addFooter(slide, n) {
  addText(slide, String(n).padStart(2, "0"), 1186, 652, 48, 28, {
    size: 14,
    color: C.muted,
    align: "right",
  });
}

function title(slide, text, subtitle, n) {
  addText(slide, text, 64, 42, 1040, 104, { size: 42, bold: true });
  if (subtitle) addText(slide, subtitle, 66, 150, 1020, 44, { size: 20, color: C.muted });
  addFooter(slide, n);
}

function panel(slide, x, y, w, h, fill = C.faint) {
  return slide.shapes.add({
    geometry: "rect",
    position: { left: x, top: y, width: w, height: h },
    fill,
    line: { style: "solid", fill: "#D4D7DC", width: 1 },
  });
}

function chip(slide, text, x, y, w, color) {
  const s = slide.shapes.add({
    geometry: "roundRect",
    position: { left: x, top: y, width: w, height: 34 },
    fill: color,
    line: { style: "solid", fill: color, width: 1 },
    borderRadius: "rounded-xl",
  });
  s.text = text;
  s.text.style = { fontSize: 15, bold: true, color: C.white, alignment: "center" };
}

function bulletList(slide, items, x, y, w, rowH = 54, size = 21) {
  items.forEach((item, i) => {
    const yy = y + i * rowH;
    slide.shapes.add({
      geometry: "ellipse",
      position: { left: x, top: yy + 10, width: 10, height: 10 },
      fill: C.blue,
      line: { style: "solid", fill: C.blue, width: 0 },
    });
    addText(slide, item, x + 24, yy, w - 24, rowH, { size, color: C.ink });
  });
}

function addArrow(slide, x1, y1, x2, y2, color = C.rule) {
  slide.shapes.add({
    geometry: "line",
    position: { left: x1, top: y1, width: x2 - x1, height: y2 - y1 },
    line: { style: "solid", fill: color, width: 3, endArrowType: "triangle" },
    fill: "none",
  });
}

function smallNode(slide, label, x, y, color = C.blue) {
  const s = slide.shapes.add({
    geometry: "ellipse",
    position: { left: x, top: y, width: 56, height: 56 },
    fill: color,
    line: { style: "solid", fill: "#222222", width: 1 },
  });
  s.text = label;
  s.text.style = { fontSize: 18, bold: true, color: C.white, alignment: "center" };
}

function routeBox(slide, label, sub, x, y, w, color) {
  panel(slide, x, y, w, 92, "#F7F8FA");
  chip(slide, label, x + 18, y + 18, 90, color);
  addText(slide, sub, x + 126, y + 20, w - 148, 52, { size: 20 });
}

function drawTable(slide, rows, x, y, w, h, widths, fontSize = 19) {
  const rowH = h / rows.length;
  const cols = rows[0].length;
  for (let r = 0; r < rows.length; r++) {
    let xx = x;
    for (let c = 0; c < cols; c++) {
      const cw = w * widths[c];
      slide.shapes.add({
        geometry: "rect",
        position: { left: xx, top: y + r * rowH, width: cw, height: rowH },
        fill: r === 0 ? C.ink : C.white,
        line: { style: "solid", fill: C.rule, width: 1 },
      });
      addText(slide, rows[r][c], xx + 12, y + r * rowH + 10, cw - 24, rowH - 16, {
        size: r === 0 ? fontSize : fontSize - 1,
        bold: r === 0,
        color: r === 0 ? C.white : C.ink,
      });
      xx += cw;
    }
  }
}

async function main() {
  await fs.mkdir("C:/Users/tomat/source/repos/I wanna Siv3D/OpenSiv3D_0.6.161/outputs", { recursive: true });
  const p = Presentation.create({ slideSize: { width: W, height: H } });

  // 1
  {
    const s = p.slides.add(); s.background.fill = C.white;
    addText(s, "Encoding NP-Complete Problems as Bullet-Hell Game Gimmicks", 64, 92, 960, 176, { size: 55, bold: true });
    addText(s, "A research presentation using an I Wanna-style engine built with Siv3D", 68, 296, 820, 54, { size: 24, color: C.muted });
    panel(s, 910, 100, 260, 420, "#F2F7FB");
    addText(s, "3SAT", 950, 152, 180, 52, { size: 42, bold: true, color: C.blue });
    addText(s, "+", 1014, 244, 60, 48, { size: 42, bold: true, align: "center" });
    addText(s, "Hamilton\nPath", 950, 326, 190, 100, { size: 38, bold: true, color: C.green });
    addText(s, "20-minute talk / 10-minute Q&A", 68, 585, 520, 34, { size: 20, color: C.muted });
    addFooter(s, 1);
  }

  // 2
  {
    const s = p.slides.add(); s.background.fill = C.white;
    title(s, "The research question is broader than difficulty", "The goal is to connect formal problem structure with playable level structure.", 2);
    bulletList(s, [
      "Can a Boolean formula or graph instance be transformed into a playable gimmick?",
      "Does stage clearability correspond to the existence of a valid solution?",
      "Can the player experience search, constraint satisfaction, and backtracking as game actions?",
      "Can the same engine support both prototype demonstrations and practical stage design?"
    ], 92, 214, 1040, 66, 24);
  }

  // 3
  {
    const s = p.slides.add(); s.background.fill = C.white;
    title(s, "Two completed prototypes give the research a concrete base", "Both reductions currently exist as provisional gimmicks; 3SAT also has an in-stage usage example.", 3);
    panel(s, 72, 202, 350, 310); panel(s, 466, 202, 350, 310); panel(s, 860, 202, 350, 310);
    addText(s, "3SAT gimmick", 104, 238, 280, 38, { size: 27, bold: true });
    addText(s, "A route-choice structure represents variable assignments. Clause sections test whether at least one literal is satisfied.", 104, 304, 272, 134, { size: 21, color: C.muted });
    addText(s, "Hamilton Path gimmick", 498, 238, 280, 74, { size: 27, bold: true });
    addText(s, "Graph traversal is mapped to player movement while enforcing edge validity and one-time vertex visits.", 498, 334, 272, 110, { size: 21, color: C.muted });
    addText(s, "Practical 3SAT example", 892, 238, 280, 74, { size: 27, bold: true });
    addText(s, "The 3SAT mechanism has moved beyond a toy prototype and has been used in an actual stage context.", 892, 334, 272, 110, { size: 21, color: C.muted });
  }

  // 4
  {
    const s = p.slides.add(); s.background.fill = C.white;
    title(s, "3SAT becomes a sequence of commitments and checks", "The player chooses truth values, then the stage tests clause satisfaction.", 4);
    routeBox(s, "TRUE", "Upper route commits the variable to true", 96, 224, 470, C.green);
    routeBox(s, "FALSE", "Lower route commits the variable to false", 96, 348, 470, C.red);
    addArrow(s, 590, 286, 700, 286, C.blue);
    addArrow(s, 590, 410, 700, 410, C.blue);
    panel(s, 730, 228, 420, 220, "#F7F8FA");
    addText(s, "Clause gate", 770, 262, 320, 42, { size: 29, bold: true });
    addText(s, "(x1 OR not x2 OR x3)", 770, 324, 320, 36, { size: 23, color: C.blue });
    addText(s, "Pass if at least one selected literal opens the path.", 770, 386, 322, 48, { size: 20, color: C.muted });
  }

  // 5
  {
    const s = p.slides.add(); s.background.fill = C.white;
    title(s, "A 3SAT stage can preserve the logical meaning of the formula", "The important claim is correspondence, not just visual analogy.", 5);
    const rows = [
      ["Formula element", "Game element", "Why it matters"],
      ["Variable", "A forced route choice", "The player commits to one truth value"],
      ["Literal", "A condition tied to a route", "Positive and negated choices can be distinguished"],
      ["Clause", "A pass/fail gate", "At least one satisfied literal opens progression"],
      ["Satisfying assignment", "A complete clear route", "Clearability mirrors satisfiability"]
    ];
    drawTable(s, rows, 84, 190, 1110, 368, [0.24, 0.31, 0.45], 21);
  }

  // 6
  {
    const s = p.slides.add(); s.background.fill = C.white;
    title(s, "The existing 3SAT use case turns the prototype into evidence", "A practical stage example lets the study discuss playability, not just construction.", 6);
    panel(s, 82, 204, 510, 322, "#F7F8FA");
    addText(s, "What the example can show", 120, 242, 420, 40, { size: 29, bold: true });
    bulletList(s, [
      "Players can read the choice structure during play",
      "The constraint checks can be embedded into action timing",
      "The mechanism can coexist with ordinary I Wanna hazards"
    ], 122, 318, 400, 62, 21);
    panel(s, 666, 204, 510, 322, "#F7F8FA");
    addText(s, "What should be measured next", 704, 242, 420, 40, { size: 29, bold: true });
    bulletList(s, [
      "Time to infer the rule",
      "Failure patterns and backtracking behavior",
      "Whether solving feels strategic or arbitrary"
    ], 706, 318, 400, 62, 21);
  }

  // 7
  {
    const s = p.slides.add(); s.background.fill = C.white;
    title(s, "Hamilton Path maps graph traversal to route planning", "The player must visit every vertex once while following only valid edges.", 7);
    addArrow(s, 278, 302, 450, 220, C.rule); addArrow(s, 278, 302, 450, 390, C.rule);
    addArrow(s, 506, 220, 696, 300, C.rule); addArrow(s, 506, 390, 696, 300, C.rule);
    addArrow(s, 752, 300, 924, 250, C.rule); addArrow(s, 752, 300, 924, 404, C.rule);
    smallNode(s, "A", 222, 274, C.blue); smallNode(s, "B", 450, 192, C.green);
    smallNode(s, "C", 450, 362, C.green); smallNode(s, "D", 696, 272, C.amber);
    smallNode(s, "E", 924, 222, C.red); smallNode(s, "F", 924, 376, C.red);
    addText(s, "A valid clear route is a Hamilton path: each vertex appears exactly once.", 164, 556, 900, 42, { size: 25, bold: true });
  }

  // 8
  {
    const s = p.slides.add(); s.background.fill = C.white;
    title(s, "The Hamilton Path gimmick has two core enforcement problems", "The stage must prevent illegal edges and repeated vertex visits.", 8);
    panel(s, 90, 214, 500, 270, "#F7F8FA"); panel(s, 690, 214, 500, 270, "#F7F8FA");
    addText(s, "Edge validity", 130, 252, 360, 42, { size: 31, bold: true });
    addText(s, "Movement options should exist only where the original graph has an edge. Non-edges must be blocked by geometry, hazards, or state.", 130, 320, 390, 120, { size: 22, color: C.muted });
    addText(s, "Visit uniqueness", 730, 252, 360, 42, { size: 31, bold: true });
    addText(s, "After a vertex is used, the corresponding route should become unavailable or fatal, forcing a one-time visit constraint.", 730, 320, 390, 120, { size: 22, color: C.muted });
  }

  // 9
  {
    const s = p.slides.add(); s.background.fill = C.white;
    title(s, "Both gimmicks express search through player movement", "This shared structure is the basis for comparing them as research objects.", 9);
    const rows = [
      ["Aspect", "3SAT", "Hamilton Path"],
      ["Choice unit", "Truth assignment", "Next vertex"],
      ["Constraint", "Every clause must be satisfied", "Every vertex must be visited once"],
      ["Failure mode", "An unsatisfied clause blocks progress", "A repeated vertex or missing edge blocks progress"],
      ["Player experience", "Commit, test, revise", "Plan, traverse, avoid reuse"]
    ];
    drawTable(s, rows, 76, 184, 1128, 388, [0.2, 0.38, 0.42], 20);
  }

  // 10
  {
    const s = p.slides.add(); s.background.fill = C.white;
    title(s, "The research contribution can be framed at three levels", "This keeps the talk balanced between theory, implementation, and game design.", 10);
    panel(s, 72, 226, 330, 250, "#F7F8FA"); panel(s, 474, 226, 330, 250, "#F7F8FA"); panel(s, 876, 226, 330, 250, "#F7F8FA");
    addText(s, "Computational", 104, 260, 260, 36, { size: 27, bold: true, color: C.blue });
    addText(s, "Clearability can be related to satisfiability or path existence.", 104, 334, 248, 86, { size: 22, color: C.muted });
    addText(s, "Educational", 506, 260, 260, 36, { size: 27, bold: true, color: C.green });
    addText(s, "Abstract NP-complete problems become visible and playable.", 506, 334, 248, 86, { size: 22, color: C.muted });
    addText(s, "Design", 908, 260, 260, 36, { size: 27, bold: true, color: C.red });
    addText(s, "Problem instances can inspire systematic stage-generation rules.", 908, 334, 248, 86, { size: 22, color: C.muted });
  }

  // 11
  {
    const s = p.slides.add(); s.background.fill = C.white;
    title(s, "Evaluation should separate correctness from playability", "A stage can be logically faithful and still fail as a game experience.", 11);
    const rows = [
      ["Criterion", "Question to answer", "Possible evidence"],
      ["Formal correspondence", "Does a solution exist iff the stage is clearable?", "Small proofs and generated test cases"],
      ["Readability", "Can players infer what each route means?", "Observation and post-play explanation"],
      ["Difficulty quality", "Is the challenge strategic rather than obscure?", "Failure logs and retry behavior"],
      ["Scalability", "What instance sizes remain playable?", "Stage size, object count, completion time"]
    ];
    drawTable(s, rows, 66, 188, 1148, 390, [0.22, 0.36, 0.42], 19);
  }

  // 12
  {
    const s = p.slides.add(); s.background.fill = C.white;
    title(s, "Next: turn examples into a repeatable method", "The prototypes are ready enough to support a research plan.", 12);
    const xs = [124, 414, 704, 994];
    const labels = [
      ["Define", "Formalize the mapping from problem instances to stage components."],
      ["Generate", "Build tools that produce candidate stages from formulas or graphs."],
      ["Validate", "Check correspondence between solution existence and clearability."],
      ["Test", "Study whether players understand and enjoy the resulting challenges."]
    ];
    xs.forEach((x, i) => {
      if (i < 3) addArrow(s, x + 116, 354, xs[i + 1] - 34, 354, C.blue);
      smallNode(s, String(i + 1), x, 326, [C.blue, C.green, C.amber, C.red][i]);
      addText(s, labels[i][0], x - 38, 418, 132, 32, { size: 25, bold: true, align: "center" });
      addText(s, labels[i][1], x - 86, 464, 228, 92, { size: 19, color: C.muted, align: "center" });
    });
  }

  // 13
  {
    const s = p.slides.add(); s.background.fill = C.white;
    addText(s, "Discussion", 64, 72, 760, 76, { size: 56, bold: true });
    addText(s, "Key takeaway", 66, 200, 220, 34, { size: 23, bold: true, color: C.blue });
    addText(s, "The prototypes suggest that NP-complete problems can become playable constraint systems, not just metaphors for difficulty.", 66, 246, 980, 92, { size: 32, bold: true });
    addText(s, "Questions to invite", 66, 426, 260, 34, { size: 23, bold: true, color: C.green });
    bulletList(s, [
      "What level of formal proof is needed for a game-based reduction?",
      "How should unsatisfiable or impossible instances be presented to players?",
      "Where is the boundary between mathematical structure and game-design fun?"
    ], 68, 480, 980, 50, 22);
    addFooter(s, 13);
  }

  const montage = await p.export({ format: "webp", montage: true, scale: 1 });
  await fs.writeFile(QA, new Uint8Array(await montage.arrayBuffer()));
  const pptx = await PresentationFile.exportPptx(p);
  await pptx.save(OUT);
}

main().catch((err) => {
  console.error(err);
  process.exitCode = 1;
});
