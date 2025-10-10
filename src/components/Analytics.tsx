import React from 'react';
import { Typography, Box } from '@mui/material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer, BarChart, Bar } from 'recharts';

const data = [
  { name: 'Jan', alerts: 20, recordings: 100 },
  { name: 'Feb', alerts: 30, recordings: 120 },
  { name: 'Mar', alerts: 25, recordings: 110 },
  { name: 'Apr', alerts: 35, recordings: 130 },
];

const Analytics: React.FC = () => {
  return (
    <div>
      <Typography variant="h4" gutterBottom>
        Analytics
      </Typography>
      <Box display="flex" flexDirection="column" gap={3}>
        <Box>
          <Typography variant="h6">Alerts Over Time</Typography>
          <ResponsiveContainer width="100%" height={300}>
            <LineChart data={data}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="name" />
              <YAxis />
              <Tooltip />
              <Legend />
              <Line type="monotone" dataKey="alerts" stroke="#8884d8" />
            </LineChart>
          </ResponsiveContainer>
        </Box>
        <Box>
          <Typography variant="h6">Recordings Over Time</Typography>
          <ResponsiveContainer width="100%" height={300}>
            <BarChart data={data}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="name" />
              <YAxis />
              <Tooltip />
              <Legend />
              <Bar dataKey="recordings" fill="#82ca9d" />
            </BarChart>
          </ResponsiveContainer>
        </Box>
      </Box>
    </div>
  );
};

export default Analytics;