export interface Note {
  el: HTMLDivElement;
  y: number;
  lane: number;
  isBlue: boolean; // Track if note is blue or red
}

export interface GameState {
  score: number;
  combo: number;
  maxCombo: number;
  secondsLeft: number;
}

// Direction keys only (0-3 for the 4 directions)
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
  noteClassName: string,
  isBlue: boolean
): Note | null {
  if (!lane) return null;

  const note = document.createElement("div");
  note.className = noteClassName;
  note.setAttribute("data-lane", laneIndex.toString());
  note.setAttribute("data-color", isBlue ? "blue" : "red");
  lane.appendChild(note);

  return { el: note, y: -50, lane: laneIndex, isBlue };
}

export function updateNotes(
  notes: Note[],
  gameState: GameState,
  onComboBreak: () => void,
  speed: number
): Note[] {
  const updatedNotes: Note[] = [];

  for (let i = 0; i < notes.length; i++) {
    const note = notes[i];
    note.y += speed;
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

// Calculate note speed based on elapsed time
export function calculateNoteSpeed(secondsElapsed: number): number {
  const baseSpeed = 4;
  const maxSpeed = 8;
  const speedIncreaseRate = 0.08; // Speed increases by 0.08 per second
  
  const calculatedSpeed = baseSpeed + (secondsElapsed * speedIncreaseRate);
  return Math.min(calculatedSpeed, maxSpeed);
}

// Calculate spawn interval based on elapsed time
export function calculateSpawnInterval(secondsElapsed: number): number {
  const baseInterval = 700; // Start at 700ms
  const minInterval = 350; // Fastest spawn rate (350ms)
  const decreaseRate = 7; // Decrease by 7ms per second
  
  const calculatedInterval = baseInterval - (secondsElapsed * decreaseRate);
  return Math.max(calculatedInterval, minInterval);
}

export function hitNote(
  laneIndex: number,
  isBluePressed: boolean,
  notes: Note[],
  gameState: GameState,
  onScoreUpdate: (score: number) => void,
  onComboUpdate: () => void,
  noteHitClassName: string
): Note[] {
  for (let i = 0; i < notes.length; i++) {
    const note = notes[i];
    
    // Check if the note matches both direction AND color modifier
    if (note.lane === laneIndex && note.isBlue === isBluePressed && note.y > 700 && note.y < 800) {
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