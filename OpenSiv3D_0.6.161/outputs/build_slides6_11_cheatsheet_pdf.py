from reportlab.pdfgen import canvas
from reportlab.lib.pagesizes import A4
from reportlab.lib.colors import HexColor
from reportlab.pdfbase.pdfmetrics import stringWidth

ROOT = r"C:\Users\tomat\source\repos\RyotaIzumi\I-wanna-Siv3D\OpenSiv3D_0.6.161\outputs"
OUT = ROOT + r"\slides6_11_english_cheatsheet_one_page.pdf"

items = [
    ("6  Constraint workflow", "Four steps: describe possible safe routes; encode the rules as 3SAT or a graph; solve for a satisfying assignment or Hamilton path; convert the solution into timed stage elements. The solver is a scaffold, not a replacement for creative design: it protects at least one valid route while we control the music and presentation."),
    ("7  3SAT prototype", "The player enters TRUE or FALSE before the timer ends. The game then checks each clause. A clause is passable when at least one of its three literals is true. Boolean assignments become movement decisions; clause satisfaction becomes a playable obstacle check."),
    ("8  Why the mapping matters", "The goal is formal correspondence, not visual similarity. Variables become forced choices, literals become route-linked conditions, and clauses become pass-or-fail gates. If the mapping is correct, the stage is clearable exactly when the formula is satisfiable. Proving and testing this relationship is a central research task."),
    ("9  LIVE DEMO", "Switch to the game. Show a timed variable assignment and the resulting clause checks. Intentionally fail once; replay with a satisfying assignment and clear the sequence. Ask whether the connection between the logical choice and the game response is understandable. Then return to the slides."),
    ("10  Hamilton Path prototype", "Each yellow platform is a vertex; permitted transitions are graph edges. Visit every platform exactly once, then reach the green goal. Repeated visits and invalid transitions must be blocked or punished. A Hamilton path supplies a complete safe route. Next: integrate this prototype into a real barrage."),
    ("11  Closing and next steps", "Implement a practical Hamilton Path example; improve the 3SAT example's readability, pacing, difficulty, and music synchronization; evaluate logical correctness and playability. Central question: can formal constraints create expressive bullet hells without losing a guaranteed route? Invite feedback on both the formal model and game design."),
]

def wrap(text, font, size, width):
    words=text.split(); lines=[]; line=""
    for word in words:
        test=(line+" "+word).strip()
        if stringWidth(test,font,size)<=width: line=test
        else:
            if line: lines.append(line)
            line=word
    if line: lines.append(line)
    return lines

w,h=A4; c=canvas.Canvas(OUT,pagesize=A4)
c.setFillColor(HexColor("#FFFFFF")); c.rect(0,0,w,h,fill=1,stroke=0)
margin=34; gap=24; colw=(w-2*margin-gap)/2
c.setFillColor(HexColor("#1F4D78")); c.setFont("Helvetica-Bold",16)
c.drawString(margin,h-38,"Slides 6-11 | English Presenter Cheat Sheet")
c.setFillColor(HexColor("#555A61")); c.setFont("Helvetica",8.5)
c.drawString(margin,h-52,"3SAT and Hamilton Path for safer bullet-hell generation - one-page print edition")
c.setStrokeColor(HexColor("#CBD2DA")); c.line(margin,h-60,w-margin,h-60)

top=h-78; bottom=38
for col in range(2):
    x=margin+col*(colw+gap); y=top
    for title,body in items[col*3:(col+1)*3]:
        c.setFillColor(HexColor("#2E74B5")); c.setFont("Helvetica-Bold",12.5)
        c.drawString(x,y,title); y-=16
        c.setFillColor(HexColor("#111111")); c.setFont("Helvetica",10.7)
        for line in wrap(body,"Helvetica",10.7,colw):
            c.drawString(x,y,line); y-=14.2
        y-=14
    if col==0:
        c.setStrokeColor(HexColor("#D8DDE3")); c.line(margin+colw+gap/2,bottom,margin+colw+gap/2,top+5)

c.setStrokeColor(HexColor("#CBD2DA")); c.line(margin,29,w-margin,29)
c.setFillColor(HexColor("#777777")); c.setFont("Helvetica-Oblique",7.5)
c.drawCentredString(w/2,17,"Speaking cue: pause after each key claim and emphasize the words in the slide title.")
c.showPage(); c.save(); print(OUT)
