// StartPage.js
import React from 'react';

const StartPage = ({ onStart }) => {
  return (
    <div className="start-page">
      <h1>Welcome to the Start Page</h1>
      <button onClick={onStart}>Start</button>
    </div>
  );
};

export default StartPage;