export interface Note {
  el: HTMLDivElement;
  y: number;
  lane: number;
  isBlue: boolean;
}

export interface GameState {
  score: number;
  combo: number;
  maxCombo: number;
  secondsLeft: number;
}

export interface GameRefs {
  scoreRef: React.RefObject<HTMLDivElement>;
  comboRef: React.RefObject<HTMLDivElement>;
  timerRef: React.RefObject<HTMLDivElement>;
}

// Constants
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

// Input constants
export const NONE = 0x00;
export const UP = 0x01;
export const DOWN = 0x02;
export const LEFT = 0x03;
export const RIGHT = 0x04;
export const RED_BUTTON = 0x00;
export const BLUE_BUTTON = 0x10;
export const FLAG_ZERO = 0x00;
export const FLAG_ONE = 0x80;

// Direction to lane mapping
export const DIRECTION_TO_LANE: { [key: number]: number } = {
  [LEFT]: 0,
  [DOWN]: 1,
  [UP]: 2,
  [RIGHT]: 3,
};

// Display update functions
export function updateComboDisplay(gameState: GameState, comboRef: React.RefObject<HTMLDivElement>) {
  if (!comboRef.current) return;
  comboRef.current.textContent = getComboText(gameState.combo);
}

export function updateTimerDisplay(gameState: GameState, timerRef: React.RefObject<HTMLDivElement>) {
  if (!timerRef.current) return;
  timerRef.current.textContent = formatTime(gameState.secondsLeft);
}

export function updateScoreDisplay(score: number, scoreRef: React.RefObject<HTMLDivElement>) {
  if (!scoreRef.current) return;
  scoreRef.current.textContent = score.toString();
}

// Note creation
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

// Spawn note function
export function spawnNote(
  lanes: (HTMLDivElement | null)[],
  notes: Note[],
  noteClassName: string
): Note[] {
  const laneIndex = Math.floor(Math.random() * 4);
  const isBlue = Math.random() < 0.5;
  const lane = lanes[laneIndex];
  
  if (!lane) return notes;

  const note = createNote(laneIndex, lane, noteClassName, isBlue);
  if (note) {
    return [...notes, note];
  }
  
  return notes;
}

// Update notes
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
  const baseSpeed = 3;
  const maxSpeed = 6;
  const speedIncreaseRate = 0.05;
  
  const calculatedSpeed = baseSpeed + (secondsElapsed * speedIncreaseRate);
  return Math.min(calculatedSpeed, maxSpeed);
}

// Calculate spawn interval based on elapsed time
export function calculateSpawnInterval(secondsElapsed: number): number {
  const baseInterval = 700;
  const minInterval = 350;
  const decreaseRate = 7;
  
  const calculatedInterval = baseInterval - (secondsElapsed * decreaseRate);
  return Math.max(calculatedInterval, minInterval);
}

// Hit note
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
    
    if (note.lane === laneIndex && note.isBlue === isBluePressed && note.y > 550 && note.y < 800) {
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

// Input parsing
export function parseInput(value: number): {
  direction: number;
  color: number;
  flag: number;
  laneIndex: number | undefined;
  isBluePressed: boolean;
} {
  const direction = value & 0x0F;
  const color = value & 0x10;
  const flag = value & 0x80;
  const laneIndex = DIRECTION_TO_LANE[direction];
  const isBluePressed = color === BLUE_BUTTON;

  return { direction, color, flag, laneIndex, isBluePressed };
}

// Get active mode from input
export function getActiveMode(value: number): 'red' | 'blue' {
  const color = value & 0x10;
  return color === RED_BUTTON ? 'red' : 'blue';
}

// Get active lanes from input
export function getActiveLanes(value: number): Set<number> {
  const direction = value & 0x0F;
  const laneIndex = DIRECTION_TO_LANE[direction];

  if (laneIndex !== undefined && direction !== NONE) {
    return new Set([laneIndex]);
  }

  return new Set();
}

// Format time
export function formatTime(seconds: number): string {
  const minutes = Math.floor(seconds / 60);
  const secs = seconds % 60;
  return `${minutes}:${secs.toString().padStart(2, "0")}`;
}

// Get combo text
export function getComboText(combo: number): string {
  return combo > 0 ? `${combo}x COMBO` : "";
}