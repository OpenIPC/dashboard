import React, { useState } from 'react';
import { Typography, Box, List, ListItem, ListItemText, Button } from '@mui/material';
import { CloudUpload, CloudDownload } from '@mui/icons-material';

interface FileItem {
  name: string;
  size: string;
  date: string;
}

const FileManager: React.FC = () => {
  const [files] = useState<FileItem[]>([
    { name: 'recording1.mp4', size: '100MB', date: '2023-10-01' },
    { name: 'majestic.yaml', size: '5KB', date: '2023-10-02' },
  ]);

  return (
    <Box>
      <Typography variant="h4" gutterBottom>
        File Manager
      </Typography>
      <Box mb={2}>
        <Button startIcon={<CloudUpload />} variant="contained">Upload</Button>
        <Button startIcon={<CloudDownload />} variant="outlined" sx={{ ml: 2 }}>Download</Button>
      </Box>
      <List>
        {files.map((file, index) => (
          <ListItem key={index}>
            <ListItemText primary={file.name} secondary={`Size: ${file.size} | Date: ${file.date}`} />
          </ListItem>
        ))}
      </List>
    </Box>
  );
};

export default FileManager;