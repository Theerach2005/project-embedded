"use client";
import React, { useEffect, useRef, useState } from "react";
import Link from "next/link";
import { database, ref, push, set, onValue, off } from "@/lib/firebase";
import {
  Note,
  GameState,
  KEYS,
  createNote,
  updateNotes,
  hitNote,
  formatTime,
  getComboText,
  calculateNoteSpeed,
  calculateSpawnInterval,
} from "@/lib/gamelogic";

import styles from './game.module.css';

interface InputData {
  key: string;
  type: 'keydown' | 'keyup';
  timestamp: number;
  mode?: 'red' | 'blue' | null;
  laneIndex?: number;
}

export default function GamePage() {
  const laneRefs = useRef<(HTMLDivElement | null)[]>([]);
  const scoreRef = useRef<HTMLDivElement | null>(null);
  const comboRef = useRef<HTMLDivElement | null>(null);
  const timerRef = useRef<HTMLDivElement | null>(null);

  const [gameOver, setGameOver] = useState(false);
  const [finalScore, setFinalScore] = useState(0);
  const [highestCombo, setHighestCombo] = useState(0);
  const [timeLeft, setTimeLeft] = useState(60);
  
  // New state for score submission
  const [scoreSubmitted, setScoreSubmitted] = useState(false);
  
  // State for tracking which mode is active (red/blue) and which lane
  const [activeMode, setActiveMode] = useState<'red' | 'blue' | null>(null);
  const [activeLanes, setActiveLanes] = useState<Set<number>>(new Set());

  // Firebase: Store received inputs for display
  const [receivedInputs, setReceivedInputs] = useState<InputData[]>([]);
  
  // Generate unique player ID
  const playerIdRef = useRef<string>(`player_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`);

  const restartGame = () => {
    setGameOver(false);
    setFinalScore(0);
    setHighestCombo(0);
    setTimeLeft(60);
    setScoreSubmitted(false);
    setActiveMode(null);
    setActiveLanes(new Set());

    if (scoreRef.current) {
      scoreRef.current.textContent = "0";
    }
    if (comboRef.current) {
      comboRef.current.textContent = "";
    }
    if (timerRef.current) {
      timerRef.current.textContent = formatTime(60);
    }
  };

  // --- FIREBASE: SEND INPUT TO DATABASE ---
  const sendInputToFirebase = async (inputData: InputData) => {
    try {
      const inputsRef = ref(database, `game_inputs/${playerIdRef.current}`);
      const newInputRef = push(inputsRef);
      
      await set(newInputRef, {
        ...inputData,
        playerId: playerIdRef.current,
        timestamp: Date.now()
      });
      
      console.log("Input sent to Firebase:", inputData);
    } catch (error) {
      console.error("Error sending input to Firebase:", error);
    }
  };

  // --- FIREBASE: GET INPUTS FROM DATABASE ---
  useEffect(() => {
    if (gameOver) return;

    const inputsRef = ref(database, `game_inputs/${playerIdRef.current}`);
    
    // Listen for real-time updates
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
        setReceivedInputs(inputArray.slice(0, 10));
        
        console.log("Received inputs from Firebase:", inputArray.length);
      }
    });

    return () => {
      off(inputsRef);
    };
  }, [gameOver]);

  // --- START FIREBASE SCORE SUBMISSION LOGIC (Realtime Database) ---
  const submitScore = async () => {
    // Prevent re-submission or submission of zero score
    if (scoreSubmitted || finalScore === 0) return;

    if (finalScore <= 0) {
        alert("Score too low to submit.");
        return;
    }

    try {
        const scoresRef = ref(database, "highscores");
        const newScoreRef = push(scoresRef);
        
        await set(newScoreRef, {
            score: finalScore,
            highestCombo: highestCombo,
            timestamp: new Date().toISOString(),
            playerId: playerIdRef.current
        });

        setScoreSubmitted(true);

    } catch (e) {
      console.error("Error submitting score:", e);
      alert("Failed to submit score. Check the console for details.");
    }
  };
  // --- END FIREBASE SCORE SUBMISSION LOGIC ---

  useEffect(() => {
    if (gameOver) return;

    const lanes = laneRefs.current;
    let notes: Note[] = [];
    const gameState: GameState = {
      score: 0,
      combo: 0,
      maxCombo: 0,
      secondsLeft: 60,
    };

    const keysPressed = new Set<string>();
    const directionKeysPressed = new Set<string>();
    
    let currentMode: 'red' | 'blue' | null = null;
    
    let currentSpeed = 4;
    let spawnIntervalId: NodeJS.Timeout | null = null;

    function updateComboDisplay() {
      if (!comboRef.current) return;
      comboRef.current.textContent = getComboText(gameState.combo);
    }

    function updateTimerDisplay() {
      if (!timerRef.current) return;
      timerRef.current.textContent = formatTime(gameState.secondsLeft);
    }

    function updateScoreDisplay(score: number) {
      if (scoreRef.current) {
        scoreRef.current.textContent = score.toString();
      }
    }

    function updateActiveLanes() {
      const newActiveLanes = new Set<number>();
      directionKeysPressed.forEach(key => {
        const laneIndex = KEYS[key];
        if (laneIndex !== undefined) {
          newActiveLanes.add(laneIndex);
        }
      });
      setActiveLanes(newActiveLanes);
    }

    updateScoreDisplay(gameState.score);
    updateComboDisplay();
    updateTimerDisplay();

    function spawnNote() {
      const laneIndex = Math.floor(Math.random() * 4);
      const isBlue = Math.random() < 0.5;
      const lane = lanes[laneIndex];
      if (!lane) return;

      const note = createNote(laneIndex, lane, styles.note, isBlue); 
      if (note) {
        notes.push(note);
      }
    }

    function update() {
      const secondsElapsed = 60 - gameState.secondsLeft;
      currentSpeed = calculateNoteSpeed(secondsElapsed);
      notes = updateNotes(notes, gameState, updateComboDisplay, currentSpeed);
    }

    function updateSpawnRate() {
      const secondsElapsed = 60 - gameState.secondsLeft;
      const newInterval = calculateSpawnInterval(secondsElapsed);
      
      if (spawnIntervalId) {
        clearInterval(spawnIntervalId);
      }
      spawnIntervalId = setInterval(spawnNote, newInterval);
    }

    function checkHit(directionKey: string) {
      const laneIndex = KEYS[directionKey];
      if (laneIndex === undefined) return;

      if (currentMode === null) return;

      const isBluePressed = currentMode === 'blue';

      notes = hitNote(
        laneIndex,
        isBluePressed,
        notes,
        gameState,
        updateScoreDisplay,
        updateComboDisplay,
        styles.noteHit
      );
    }

    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.repeat) return;

      const key = e.key.toLowerCase();
      keysPressed.add(key);

      if (key === 'x') {
        currentMode = 'red';
        setActiveMode('red');
        
        // Send X key press to Firebase
        sendInputToFirebase({
          key: 'x',
          type: 'keydown',
          timestamp: Date.now(),
          mode: 'red'
        });
      } else if (key === 'c') {
        currentMode = 'blue';
        setActiveMode('blue');
        
        // Send C key press to Firebase
        sendInputToFirebase({
          key: 'c',
          type: 'keydown',
          timestamp: Date.now(),
          mode: 'blue'
        });
      }

      // If a direction key is pressed, add to active lanes and check for hit
      if (KEYS[e.key] !== undefined) {
        const laneIndex = KEYS[e.key];
        directionKeysPressed.add(e.key);
        updateActiveLanes();
        
        // Send arrow key press to Firebase
        sendInputToFirebase({
          key: e.key,
          type: 'keydown',
          timestamp: Date.now(),
          mode: currentMode,
          laneIndex: laneIndex
        });
        
        checkHit(e.key);
      }
    };

    const handleKeyUp = (e: KeyboardEvent) => {
      const key = e.key.toLowerCase();
      keysPressed.delete(key);

      if (key === 'x' && currentMode === 'red') {
        currentMode = null;
        setActiveMode(null);
        
        // Send X key release to Firebase
        sendInputToFirebase({
          key: 'x',
          type: 'keyup',
          timestamp: Date.now(),
          mode: null
        });
      } else if (key === 'c' && currentMode === 'blue') {
        currentMode = null;
        setActiveMode(null);
        
        // Send C key release to Firebase
        sendInputToFirebase({
          key: 'c',
          type: 'keyup',
          timestamp: Date.now(),
          mode: null
        });
      }

      // If a direction key is released, remove from active lanes
      if (KEYS[e.key] !== undefined) {
        directionKeysPressed.delete(e.key);
        updateActiveLanes();
        
        // Send arrow key release to Firebase
        sendInputToFirebase({
          key: e.key,
          type: 'keyup',
          timestamp: Date.now()
        });
      }
    };

    document.addEventListener("keydown", handleKeyDown);
    document.addEventListener("keyup", handleKeyUp);

    spawnIntervalId = setInterval(spawnNote, 700);
    const updateInterval = setInterval(update, 16);
    const spawnRateUpdateInterval = setInterval(updateSpawnRate, 5000);

    const timerInterval = setInterval(() => {
      gameState.secondsLeft--;
      setTimeLeft(gameState.secondsLeft);
      updateTimerDisplay();

      if (gameState.secondsLeft <= 0) {
        setFinalScore(gameState.score);
        setHighestCombo(gameState.maxCombo);
        setGameOver(true);
      }
    }, 1000);

    return () => {
      document.removeEventListener("keydown", handleKeyDown);
      document.removeEventListener("keyup", handleKeyUp);
      if (spawnIntervalId) clearInterval(spawnIntervalId);
      clearInterval(updateInterval);
      clearInterval(spawnRateUpdateInterval);
      clearInterval(timerInterval);
      
      notes.forEach(note => {
        if (note.el && note.el.parentNode) {
          note.el.remove();
        }
      });
    };
  }, [gameOver]);


  return (
    <>
      <div className={styles.body}>
        <div className={styles.game}>
          <div
            className={`${styles.timer} ${timeLeft <= 10 ? styles.timerWarning : styles.timerNormal}`}
            ref={timerRef}
          >
            {formatTime(timeLeft)}
          </div>
          <div className={styles.score} ref={scoreRef}>
            0
          </div>
          <div className={styles.combo} ref={comboRef}></div>

          {/* Display received inputs (optional - for debugging) */}
          <div className={styles.inputDisplay}>
            {receivedInputs.slice(0, 3).map((input: any, index) => (
              <div key={input.id || index} className={styles.inputItem}>
                {input.key} {input.type === 'keydown' ? '↓' : '↑'}
              </div>
            ))}
          </div>

          {[0, 1, 2, 3].map((i) => (
            <div
              key={i}
              className={`${styles.lane} ${
                activeLanes.has(i) && activeMode === 'red' ? styles.laneRed : 
                activeLanes.has(i) && activeMode === 'blue' ? styles.laneBlue : ''
              }`}
              ref={(el) => {
                laneRefs.current[i] = el;
              }}
            ></div>
          ))}

          <div className={styles.hitbar}></div>

          {gameOver && (
            <div className={styles.gameOverOverlay}>
              <div className={styles.gameOverBox}>
                <h1 className="text-5xl font-bold mb-4 text-white">Game Over!</h1>
                <p className="text-3xl mb-4 text-white">
                  Final Score:{" "}
                  <span className="text-yellow-400 font-bold">{finalScore}</span>
                </p>
                <p className="text-2xl mb-8 text-white">
                  Highest Combo:{" "}
                  <span className="text-orange-400 font-bold">
                    {highestCombo}x
                  </span>
                </p>

                <div className="mb-8 p-4 bg-gray-800 rounded-lg">
                    <h2 className="text-xl font-semibold mb-3 text-white">Submit Score to Leaderboard</h2>
                    <button
                        onClick={submitScore}
                        disabled={scoreSubmitted || finalScore === 0}
                        className={`w-full px-6 py-3 rounded-xl text-xl font-semibold transition-all text-white 
                            ${scoreSubmitted || finalScore === 0
                                ? 'bg-gray-500 cursor-not-allowed' 
                                : 'bg-indigo-600 hover:bg-indigo-700'}`
                        }
                    >
                        {scoreSubmitted ? "Score Submitted!" : "Submit Score"}
                    </button>
                </div>

                <div className="flex gap-4 justify-center">
                  <button
                    onClick={restartGame}
                    className="px-8 py-4 bg-blue-600 hover:bg-blue-700 rounded-xl text-xl font-semibold transition-all text-white"
                  >
                    Play Again
                  </button>

                  <Link
                    href="/leaderboard"
                    className="px-8 py-4 bg-green-600 hover:bg-green-700 rounded-xl text-xl font-semibold transition-all text-white inline-block"
                  >
                    View Leaderboard
                  </Link>
                </div>
              </div>
            </div>
          )}
        </div>
      </div>
    </>
  );
}