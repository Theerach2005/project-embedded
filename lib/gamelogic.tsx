export interface Note {
  el: HTMLDivElement;
  y: number;
  lane: number;
}

export interface GameState {
  score: number;
  combo: number;
  maxCombo: number;
  secondsLeft: number;
}

export const KEYS: { [key: string]: number } = {
  ArrowLeft: 0,
  ArrowDown: 1,
  ArrowUp: 2,
  ArrowRight: 3,
  a: 0,
  s: 1,
  w: 2,
  d: 3,
  A: 0,
  S: 1,
  W: 2,
  D: 3,
};

export function createNote(
  laneIndex: number,
  lane: HTMLDivElement,
  noteClassName: string
): Note | null {
  if (!lane) return null;

  const note = document.createElement("div");
  // Use the scoped class name passed from the React component
  note.className = noteClassName;
  note.setAttribute("data-lane", laneIndex.toString());
  lane.appendChild(note);

  return { el: note, y: -50, lane: laneIndex };
}

export function updateNotes(
  notes: Note[],
  gameState: GameState,
  onComboBreak: () => void
): Note[] {
  const updatedNotes: Note[] = [];

  for (let i = 0; i < notes.length; i++) {
    const note = notes[i];
    note.y += 4;
    note.el.style.top = `${note.y}px`;

    if (note.y > 850) {
      note.el.remove();
      gameState.combo = 0;
      onComboBreak();
    } else {
      updatedNotes.push(note);
    }
  }

  return updatedNotes;
}

export function hitNote(
  laneIndex: number,
  notes: Note[],
  gameState: GameState,
  onScoreUpdate: (score: number) => void,
  onComboUpdate: () => void,
  noteHitClassName: string // New argument for the scoped hit class
): Note[] {
  for (let i = 0; i < notes.length; i++) {
    const note = notes[i];

    if (note.lane === laneIndex && note.y > 700 && note.y < 800) {
      // Use the scoped hit class name
      note.el.classList.add(noteHitClassName);
      
      gameState.score += 100;
      gameState.combo++;

      if (gameState.combo > gameState.maxCombo) {
        gameState.maxCombo = gameState.combo;
      }

      onScoreUpdate(gameState.score);
      onComboUpdate();

      setTimeout(() => note.el.remove(), 300);

      const updatedNotes = [...notes];
      updatedNotes.splice(i, 1);
      return updatedNotes;
    }
  }

  return notes;
}

export function formatTime(seconds: number): string {
  const minutes = Math.floor(seconds / 60);
  const secs = seconds % 60;
  return `${minutes}:${secs.toString().padStart(2, "0")}`;
}

export function getComboText(combo: number): string {
  return combo > 0 ? `${combo}x COMBO` : "";
}