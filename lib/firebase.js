// lib/firebase.js

// 1. Import necessary functions from the Firebase SDK
import { initializeApp } from "firebase/app";
import { 
  getFirestore, 
  collection, 
  query, 
  orderBy, 
  limit, 
  onSnapshot, // <-- This is needed for your LeaderboardPage
  addDoc,       // <-- This is needed for your GamePage (score submission)
  serverTimestamp, // <-- This is needed for your GamePage
  getDocs
} from "firebase/firestore";

// 2. Load your web app's Firebase configuration from environment variables
const firebaseConfig = {
  apiKey: process.env.NEXT_PUBLIC_FIREBASE_API_KEY,
  authDomain: process.env.NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN,
  databaseURL: process.env.NEXT_PUBLIC_FIREBASE_DATABASE_URL,
  projectId: process.env.NEXT_PUBLIC_FIREBASE_PROJECT_ID,
  storageBucket: process.env.NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET,
  messagingSenderId: process.env.NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID,
  appId: process.env.NEXT_PUBLIC_FIREBASE_APP_ID,
};

// 3. Initialize Firebase App
const app = initializeApp(firebaseConfig);

// 4. Initialize Cloud Firestore and export the database reference (db)
export const db = getFirestore(app);

// 5. Export ALL the Firestore functions needed by your application components.
// These are the members that were missing in your file.
export { 
  collection, 
  query,
  orderBy,
  limit,
  onSnapshot, 
  addDoc, 
  serverTimestamp,
  getDocs
};