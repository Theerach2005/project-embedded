"use client";
import React, { useEffect, useRef, useState, useMemo } from "react";
import Link from "next/link";
import {
  Note,
  GameState,
  updateNotes,
  formatTime,
  calculateNoteSpeed,
  calculateSpawnInterval,
  spawnNote,
  updateComboDisplay,
  updateTimerDisplay,
  updateScoreDisplay,
  parseInput,
  getActiveMode,
  getActiveLanes,
  hitNote,
  NONE,
} from "@/lib/gamelogic";

// Import backend service functions
import {
  InputData,
  generatePlayerId,
  submitScoreToFirebase,
  subscribeToInput,
} from "@/app/services/gameService";

import styles from './game.module.css';

export default function GamePage() {
  const laneRefs = useRef<(HTMLDivElement | null)[]>([]);
  const scoreRef = useRef<HTMLDivElement | null>(null);
  const comboRef = useRef<HTMLDivElement | null>(null);
  const timerRef = useRef<HTMLDivElement | null>(null);

  const [gameOver, setGameOver] = useState(false);
  const [finalScore, setFinalScore] = useState(0);
  const [highestCombo, setHighestCombo] = useState(0);
  const [timeLeft, setTimeLeft] = useState(60);
  const [scoreSubmitted, setScoreSubmitted] = useState(false);
  const [activeMode, setActiveMode] = useState<'red' | 'blue' | null>(null);
  const [input, setInput] = useState<InputData | null>(null);

  // Generate unique player ID once
  const playerIdRef = useRef<string>(generatePlayerId());

  // Refs to hold game state and notes accessible across useEffects
  const notesRef = useRef<Note[]>([]);
  const gameStateRef = useRef<GameState>({
    score: 0,
    combo: 0,
    maxCombo: 0,
    secondsLeft: 60,
  });

  const restartGame = () => {
    setGameOver(false);
    setFinalScore(0);
    setHighestCombo(0);
    setTimeLeft(60);
    setScoreSubmitted(false);

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

  // Subscribe to Firebase inputs
  useEffect(() => {
    if (gameOver) return;
    
    const unsubscribe = subscribeToInput((input: InputData | null) => {
      setInput(input);
    });

    return () => {
      unsubscribe();
    };
  }, [gameOver]);

  // Set activeMode based on input and clear after 300ms
  useEffect(() => {
    if (gameOver) {
      setActiveMode(null);
      return;
    }

    if (!input) {
      setActiveMode(null);
      return;
    }

    const mode = getActiveMode(input.value);
    setActiveMode(mode);

    const timeoutId = setTimeout(() => {
      setActiveMode(null);
    }, 300);

    return () => {
      clearTimeout(timeoutId);
    };
  }, [gameOver, input]);

  // Calculate active lanes from input
  const activeLanes: Set<number> = useMemo(() => {
    if (gameOver || !input) return new Set();
    return getActiveLanes(input.value);
  }, [gameOver, input]);

  // Handle score submission
  const submitScore = async () => {
    if (scoreSubmitted || finalScore === 0) return;

    if (finalScore <= 0) {
      alert("Score too low to submit.");
      return;
    }

    try {
      await submitScoreToFirebase({
        score: finalScore,
        highestCombo: highestCombo,
        timestamp: new Date().toISOString(),
        playerId: playerIdRef.current
      });

      setScoreSubmitted(true);
    } catch (error) {
      console.error("Error submitting score:", error);
      alert("Failed to submit score. Check the console for details.");
    }
  };

  // Main game logic
  useEffect(() => {
    if (gameOver) return;

    const lanes = laneRefs.current;
    notesRef.current = [];
    gameStateRef.current = {
      score: 0,
      combo: 0,
      maxCombo: 0,
      secondsLeft: 60,
    };

    let currentSpeed = 4;
    let spawnIntervalId: NodeJS.Timeout | null = null;

    // Initialize displays
    updateScoreDisplay(gameStateRef.current.score, scoreRef);
    updateComboDisplay(gameStateRef.current, comboRef);
    updateTimerDisplay(gameStateRef.current, timerRef);

    // Spawn notes at intervals
    const spawnNoteWrapper = () => {
      notesRef.current = spawnNote(lanes, notesRef.current, styles.note);
    };

    // Update game state
    const update = () => {
      const secondsElapsed = 60 - gameStateRef.current.secondsLeft;
      currentSpeed = calculateNoteSpeed(secondsElapsed);
      notesRef.current = updateNotes(
        notesRef.current,
        gameStateRef.current,
        () => updateComboDisplay(gameStateRef.current, comboRef),
        currentSpeed
      );
    };

    // Update spawn rate dynamically
    const updateSpawnRate = () => {
      const secondsElapsed = 60 - gameStateRef.current.secondsLeft;
      const newInterval = calculateSpawnInterval(secondsElapsed);

      if (spawnIntervalId) {
        clearInterval(spawnIntervalId);
      }
      spawnIntervalId = setInterval(spawnNoteWrapper, newInterval);
    };

    // Start intervals
    spawnIntervalId = setInterval(spawnNoteWrapper, 700);
    const updateInterval = setInterval(update, 16);
    const spawnRateUpdateInterval = setInterval(updateSpawnRate, 5000);

    // Timer countdown
    const timerInterval = setInterval(() => {
      gameStateRef.current.secondsLeft--;
      setTimeLeft(gameStateRef.current.secondsLeft);
      updateTimerDisplay(gameStateRef.current, timerRef);

      if (gameStateRef.current.secondsLeft <= 0) {
        setFinalScore(gameStateRef.current.score);
        setHighestCombo(gameStateRef.current.maxCombo);
        setGameOver(true);
      }
    }, 1000);

    // Cleanup
    return () => {
      if (spawnIntervalId) clearInterval(spawnIntervalId);
      clearInterval(updateInterval);
      clearInterval(spawnRateUpdateInterval);
      clearInterval(timerInterval);

      notesRef.current.forEach(note => {
        if (note.el && note.el.parentNode) {
          note.el.remove();
        }
      });
    };
  }, [gameOver]);

  // Capture input and trigger hitNote
  useEffect(() => {
    if (!input || gameOver) return;

    const { direction, laneIndex, isBluePressed } = parseInput(input.value);

    // Only process if we have a valid direction
    if (laneIndex !== undefined && direction !== NONE) {
      notesRef.current = hitNote(
        laneIndex,
        isBluePressed,
        notesRef.current,
        gameStateRef.current,
        (score) => updateScoreDisplay(score, scoreRef),
        () => updateComboDisplay(gameStateRef.current, comboRef),
        styles.noteHit
      );
    }
  }, [input, gameOver]);

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

          <div className={styles.inputDisplay}>
            {input && input.value}
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