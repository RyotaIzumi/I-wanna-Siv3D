import fs from "node:fs/promises";
import { FileBlob, PresentationFile } from "@oai/artifact-tool";

const input = "C:/Users/tomat/source/repos/I wanna Siv3D/OpenSiv3D_0.6.161/outputs/np_complete_bullet_hell_research.pptx";
const output = "C:/Users/tomat/source/repos/I wanna Siv3D/OpenSiv3D_0.6.161/outputs/np_complete_bullet_hell_research_with_notes.pptx";
const translationOut = "C:/Users/tomat/source/repos/I wanna Siv3D/OpenSiv3D_0.6.161/outputs/np_complete_bullet_hell_translation_and_script.txt";

const slideNotes = [
  {
    title: "Encoding NP-Complete Problems as Bullet-Hell Game Gimmicks",
    script:
      "Today I will talk about a research project that connects computational complexity with game design. The basic idea is to take NP-complete problems, especially 3SAT and Hamilton Path, and express them as playable bullet-hell gimmicks in an I Wanna-style engine built with Siv3D. I already have provisional gimmicks for both problems, and the 3SAT gimmick has also been used in an actual stage context. The goal of this talk is to explain what has been built, why it is interesting as research, and what should be evaluated next.",
    translation:
      "今日は、計算複雑性とゲームデザインをつなぐ研究について発表します。基本的なアイデアは、3SATやHamilton PathのようなNP完全問題を、Siv3D製のアイワナ風エンジン上でプレイ可能な弾幕ギミックとして表現することです。現時点で両方の仮ギミックは用意できており、3SATについては実際のステージでの使用例もあります。この発表では、何を作ったのか、なぜ研究として面白いのか、次に何を評価すべきかを説明します。"
  },
  {
    title: "The research question is broader than difficulty",
    script:
      "A common first impression is that this project is about making very difficult stages. But the research question is broader than that. I want to ask whether a formal problem instance can be transformed into a playable gimmick, whether clearability corresponds to the existence of a solution, and whether the player can experience search and constraint satisfaction through movement. In other words, the stage is not only hard; it is hard for a reason that comes from the original problem structure.",
    translation:
      "この研究は、単に難しいステージを作ることだけが目的ではありません。研究の問いはもっと広く、形式的な問題インスタンスをプレイ可能なギミックに変換できるか、ステージのクリア可能性が解の存在と対応するか、そしてプレイヤーが移動を通して探索や制約充足を体験できるか、という点にあります。つまり、ステージが難しいだけでなく、その難しさが元の問題構造に由来していることが重要です。"
  },
  {
    title: "Two completed prototypes give the research a concrete base",
    script:
      "The current work has three important pieces of evidence. First, the 3SAT prototype maps variable choices and clause checks into a route-based gimmick. Second, the Hamilton Path prototype maps graph traversal into movement constraints. Third, the 3SAT mechanism is not only a small isolated demo; it has already been used in a practical stage context. This gives the research a concrete starting point for both theoretical discussion and playability evaluation.",
    translation:
      "現状の成果には重要な要素が3つあります。1つ目は、3SATの仮ギミックで、変数選択と節の判定をルート選択型のギミックに対応させています。2つ目はHamilton Pathの仮ギミックで、グラフの走査を移動制約として表現しています。3つ目は、3SATの仕組みが単なる小さなデモではなく、実際のステージ文脈でも使われていることです。これにより、理論的な議論とプレイ性の評価の両方に進める具体的な土台があります。"
  },
  {
    title: "3SAT becomes a sequence of commitments and checks",
    script:
      "For 3SAT, the player first makes commitments. For example, taking one route can mean assigning a variable to true, while another route can mean assigning it to false. After these choices, the stage tests clauses. A clause gate is passable if at least one of its literals is satisfied by the earlier choices. This makes the formula structure visible as a sequence of decisions followed by checks.",
    translation:
      "3SATでは、プレイヤーはまず真偽値の選択を行います。たとえば、あるルートを通ることが変数をTrueにすることを意味し、別のルートを通ることがFalseにすることを意味します。その後、ステージ内で節の判定を行います。節ゲートは、先ほどの選択によって少なくとも1つのリテラルが満たされていれば通過可能になります。このように、論理式の構造を「選択」と「判定」の連続として見える形にします。"
  },
  {
    title: "A 3SAT stage can preserve the logical meaning of the formula",
    script:
      "The key point is correspondence. Variables, literals, clauses, and satisfying assignments each have a game-side meaning. If the mapping is designed correctly, a satisfying assignment should correspond to a complete clear route. This is what makes the stage a research object rather than only a themed level. The question becomes whether the game construction preserves the logic of the original formula.",
    translation:
      "ここで重要なのは対応関係です。変数、リテラル、節、充足割当のそれぞれにゲーム上の意味があります。対応が正しく設計されていれば、充足割当は完全なクリアルートに対応するはずです。これにより、ステージは単なるテーマ付きのステージではなく、研究対象になります。問題は、ゲーム内の構成が元の論理式の意味を保っているかどうかです。"
  },
  {
    title: "The existing 3SAT use case turns the prototype into evidence",
    script:
      "The practical 3SAT use case is especially important because it lets us discuss playability. A prototype can show that a construction is possible, but an actual stage can show whether players can read it, whether the timing and hazards still work, and whether the mechanism feels strategic rather than arbitrary. For the next research step, I want to measure how players infer the rule, where they fail, and whether their retries resemble problem-solving behavior.",
    translation:
      "3SATの実戦使用例は、プレイ性を議論できる点で特に重要です。プロトタイプは構成が可能であることを示せますが、実際のステージでは、プレイヤーが仕組みを読み取れるか、タイミングや弾幕として成立するか、そして理不尽ではなく戦略的に感じられるかを確認できます。次の研究段階では、プレイヤーがルールをどう推測するか、どこで失敗するか、リトライが問題解決行動に近いかを測りたいです。"
  },
  {
    title: "Hamilton Path maps graph traversal to route planning",
    script:
      "Hamilton Path gives a different kind of structure. Instead of assigning truth values, the player chooses a path through vertices. The rule is that every vertex must be visited exactly once, and movement should follow only valid edges. In game terms, the player is planning a route through rooms, nodes, or sections while avoiding illegal transitions and repeated visits.",
    translation:
      "Hamilton Pathは3SATとは異なる構造を持っています。真偽値を割り当てるのではなく、プレイヤーは頂点を通る経路を選びます。ルールは、すべての頂点をちょうど一度ずつ訪れ、存在する辺に沿ってのみ移動することです。ゲーム上では、プレイヤーが部屋やノード、区間を通るルートを計画し、不正な移動や再訪問を避ける形になります。"
  },
  {
    title: "The Hamilton Path gimmick has two core enforcement problems",
    script:
      "For Hamilton Path, two enforcement problems are central. First, the stage must represent edge validity. If two vertices are not connected in the graph, the player should not be able to move between them safely. Second, the stage must represent visit uniqueness. Once a vertex has been used, returning to it should be blocked or punished. These two constraints are what make the game route correspond to a Hamilton path.",
    translation:
      "Hamilton Pathでは、中心となる強制条件が2つあります。1つ目は辺の有効性です。グラフ上でつながっていない2つの頂点間は、安全に移動できないようにする必要があります。2つ目は訪問の一意性です。一度使った頂点に戻ることは、ブロックされるか失敗につながるべきです。この2つの制約によって、ゲーム内のルートがHamilton Pathに対応します。"
  },
  {
    title: "Both gimmicks express search through player movement",
    script:
      "Comparing the two gimmicks is useful because they express different problems through the same medium: player movement. In 3SAT, the choice unit is a truth assignment; in Hamilton Path, it is the next vertex. In 3SAT, failure comes from an unsatisfied clause; in Hamilton Path, failure comes from a missing edge or repeated vertex. This comparison can help clarify what kinds of computational structures are natural for an action game.",
    translation:
      "2つのギミックを比較することは有用です。どちらも異なる問題を、プレイヤーの移動という同じ媒体で表現しているからです。3SATでは選択単位が真偽値割当であり、Hamilton Pathでは次に訪れる頂点です。3SATでは未充足の節が失敗につながり、Hamilton Pathでは存在しない辺や頂点の再訪問が失敗につながります。この比較によって、どのような計算構造がアクションゲームに自然に向いているかを考えられます。"
  },
  {
    title: "The research contribution can be framed at three levels",
    script:
      "I think the contribution can be framed at three levels. At the computational level, the project studies whether clearability can correspond to a formal solution. At the educational level, it makes abstract NP-complete problems visible and playable. At the design level, it suggests that problem instances could become a source of systematic stage design or even stage generation.",
    translation:
      "この研究の貢献は3つのレベルで説明できます。計算理論のレベルでは、クリア可能性が形式的な解と対応するかを調べます。教育的なレベルでは、抽象的なNP完全問題を見える形、プレイできる形にします。ゲームデザインのレベルでは、問題インスタンスを体系的なステージ設計、あるいは自動生成の素材として使える可能性を示します。"
  },
  {
    title: "Evaluation should separate correctness from playability",
    script:
      "For evaluation, I want to separate correctness from playability. Correctness asks whether the construction really preserves the problem: a solution exists if and only if the stage can be cleared. Playability asks whether players can understand and enjoy the result. Both matter. A stage can be formally correct but unreadable, or fun but not faithful to the original problem. The research should evaluate both sides explicitly.",
    translation:
      "評価では、正しさとプレイ性を分けて考えたいです。正しさとは、その構成が本当に元の問題を保っているか、つまり解が存在する場合に限りステージがクリア可能かということです。プレイ性とは、プレイヤーが理解し、楽しめるかということです。どちらも重要です。形式的には正しいが読みにくいステージもあり得ますし、楽しいけれど元の問題に忠実でないステージもあり得ます。そのため、両方を明示的に評価する必要があります。"
  },
  {
    title: "Next: turn examples into a repeatable method",
    script:
      "The next step is to move from individual examples to a repeatable method. First, I want to formalize the mapping from problem instances to stage components. Then I want to generate candidate stages from formulas or graphs, validate whether the correspondence holds, and finally test whether players can understand and enjoy the resulting challenges. This would turn the current prototypes into a more systematic research project.",
    translation:
      "次の段階では、個別の例から再現可能な方法へ進めたいです。まず、問題インスタンスからステージ部品への対応を形式化します。次に、論理式やグラフから候補ステージを生成し、その対応が成り立つかを検証します。最後に、プレイヤーがそのチャレンジを理解し、楽しめるかを調べます。これにより、現在のプロトタイプをより体系的な研究へ発展させられます。"
  },
  {
    title: "Discussion",
    script:
      "To close, the main takeaway is that these prototypes suggest a way to treat NP-complete problems as playable constraint systems, not only as metaphors for difficulty. I would like to discuss how much formal proof is needed, how impossible instances should be presented to players, and where the boundary lies between mathematical structure and game-design fun.",
    translation:
      "最後に、この研究の要点は、NP完全問題を単なる難しさの比喩ではなく、プレイ可能な制約システムとして扱える可能性を示していることです。議論したい点として、どの程度の形式的証明が必要か、解のないインスタンスをプレイヤーにどう提示すべきか、数学的構造とゲームとしての面白さの境界はどこにあるのか、という問題があります。"
  }
];

function buildNoteText(note, index) {
  return [
    `Slide ${index + 1}: ${note.title}`,
    "",
    "Presenter script:",
    note.script,
    "",
    "Japanese translation:",
    note.translation
  ].join("\n");
}

function buildTranslationFile() {
  const lines = [
    "NP-Complete Bullet-Hell Research Presentation",
    "Speaker Script and Japanese Translation",
    ""
  ];
  slideNotes.forEach((note, index) => {
    lines.push(`Slide ${index + 1}: ${note.title}`);
    lines.push("");
    lines.push("[Presenter script]");
    lines.push(note.script);
    lines.push("");
    lines.push("[Japanese translation]");
    lines.push(note.translation);
    lines.push("");
    lines.push("----");
    lines.push("");
  });
  return lines.join("\n");
}

async function main() {
  const presentation = await PresentationFile.importPptx(await FileBlob.load(input));
  if (presentation.slides.items.length !== slideNotes.length) {
    throw new Error(`Expected ${slideNotes.length} slides, found ${presentation.slides.items.length}`);
  }

  presentation.slides.items.forEach((slide, index) => {
    slide.speakerNotes.clear();
    slide.speakerNotes.textFrame.setText(buildNoteText(slideNotes[index], index));
    slide.speakerNotes.setVisible(true);
  });

  const pptx = await PresentationFile.exportPptx(presentation);
  await pptx.save(output);
  await fs.writeFile(translationOut, `\uFEFF${buildTranslationFile()}`, "utf8");
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
