import { database, ref, push, set, onValue, off } from "@/lib/firebase";
import { unsubscribe } from "node:diagnostics_channel";

// export interface InputData {
//   key: string;
//   type: 'keydown' | 'keyup';
//   timestamp: number;
//   mode?: 'red' | 'blue' | null;
//   laneIndex?: number;
// }

export interface InputData {
  value: number;
}

export interface ScoreData {
  score: number;
  highestCombo: number;
  timestamp: string;
  playerId: string;
}

/**
 * Generate a unique player ID
 */
export function generatePlayerId(): string {
  return `player_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
}

/**
 * Send input data to Firebase Realtime Database
 */
export async function sendInputToFirebase(
  playerId: string,
  inputData: InputData
): Promise<void> {
  try {
    const inputsRef = ref(database, `game_inputs/${playerId}`);
    const newInputRef = push(inputsRef);

    await set(newInputRef, {
      ...inputData,
      playerId: playerId,
      timestamp: Date.now()
    });

    console.log("Input sent to Firebase:", inputData);
  } catch (error) {
    console.error("Error sending input to Firebase:", error);
    throw error;
  }
}

/**
 * Subscribe to real-time input updates from Firebase
 * Returns an unsubscribe function
 */
export function subscribeToInputs(
  playerId: string,
  callback: (inputs: any[]) => void
): () => void {
  const inputsRef = ref(database, `game_inputs/${playerId}`);

  const unsubscribe = onValue(inputsRef, (snapshot) => {
    const data = snapshot.val();
    if (data) {
      const inputArray = Object.entries(data).map(([id, value]) => ({
        id,
        ...(value as InputData)
      }));

      // Sort by timestamp (newest first)
      inputArray.sort((a: any, b: any) => b.timestamp - a.timestamp);

      // Keep only last 10 inputs
      callback(inputArray.slice(0, 10));

      console.log("Received inputs from Firebase:", inputArray.length);
    }
  });

  return () => {
    off(inputsRef);
  };
}

export function subscribeToInput(
  setInput: (input: InputData | null) => void
): () => void {
  const lastByteRef = ref(database, "/stm32/lastByte");
  const unsubscribe = onValue(lastByteRef, (snapshot) => {
    const val = snapshot.val() as InputData | null;
    setInput(val);
    console.log("New data from Firebase:", val);
  });
  return () => {
    unsubscribe();
  };
}

/**
 * Submit score to Firebase Realtime Database leaderboard
 */
export async function submitScoreToFirebase(
  scoreData: ScoreData
): Promise<void> {
  if (scoreData.score <= 0) {
    throw new Error("Score too low to submit");
  }

  try {
    const scoresRef = ref(database, "highscores");
    const newScoreRef = push(scoresRef);

    await set(newScoreRef, {
      score: scoreData.score,
      highestCombo: scoreData.highestCombo,
      timestamp: scoreData.timestamp,
      playerId: scoreData.playerId
    });

    console.log("Score submitted successfully:", scoreData);
  } catch (error) {
    console.error("Error submitting score:", error);
    throw error;
  }
}

/**
 * Clean up player's input data (optional - call when game ends)
 */
export async function cleanupPlayerInputs(playerId: string): Promise<void> {
  try {
    const inputsRef = ref(database, `game_inputs/${playerId}`);
    await set(inputsRef, null);
    console.log("Player inputs cleaned up");
  } catch (error) {
    console.error("Error cleaning up inputs:", error);
  }
}
