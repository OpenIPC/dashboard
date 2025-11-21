import React, { useCallback, useEffect, useMemo, useState } from 'react';
import {
	Box,
	Chip,
	Dialog,
	DialogContent,
	DialogTitle,
	Divider,
	IconButton,
	Stack,
	Tooltip,
	Typography,
	Tabs,
	Tab,
} from '@mui/material';
import RefreshRoundedIcon from '@mui/icons-material/RefreshRounded';
import WarningAmberRoundedIcon from '@mui/icons-material/WarningAmberRounded';
import CheckCircleRoundedIcon from '@mui/icons-material/CheckCircleRounded';
import CloseRoundedIcon from '@mui/icons-material/CloseRounded';
import SignalCellularAltIcon from '@mui/icons-material/SignalCellularAlt';
import type { StreamPathStatus } from '../types';
import { fetchStreamPathStatuses } from '../services/streamBridge';
import { useLocalization } from '../hooks/useLocalization';

interface AppStatusProps {
	open: boolean;
	onClose: () => void;
	webrtcStatsData?: Array<{
		streamName: string;
		stats: any;
		quality: string;
	}>;
}

const REFRESH_INTERVAL_MS = 15_000;

const AppStatus: React.FC<AppStatusProps> = ({ open, onClose, webrtcStatsData = [] }) => {
	const { t } = useLocalization();
	const [paths, setPaths] = useState<StreamPathStatus[]>([]);
	const [lastUpdated, setLastUpdated] = useState<number | null>(null);
	const [isRefreshing, setIsRefreshing] = useState(false);
	const [error, setError] = useState<string | null>(null);
	const [currentTab, setCurrentTab] = useState(0);

	const loadStatuses = useCallback(async () => {
		setIsRefreshing(true);
		try {
			const result = await fetchStreamPathStatuses();
			setPaths(result);
			setLastUpdated(Date.now());
			setError(null);
		} catch (err) {
			setError(err instanceof Error ? err.message : t('stream_status_fetch_error'));
		} finally {
			setIsRefreshing(false);
		}
	}, [t]);

	useEffect(() => {
		if (!open) {
			return undefined;
		}

		void loadStatuses();
		const timer = window.setInterval(() => {
			void loadStatuses();
		}, REFRESH_INTERVAL_MS);

		return () => window.clearInterval(timer);
	}, [open, loadStatuses]);

	const summary = useMemo(() => {
		const total = paths.length;
		let ready = 0;
		let readers = 0;
		const protocols = new Set<string>();

		for (const item of paths) {
			if (item.ready) {
				ready += 1;
			}
			readers += item.readerCount;
			item.activeProtocols.forEach(proto => protocols.add(proto.toUpperCase()));
		}

		return {
			total,
			ready,
			readers,
			protocols: Array.from(protocols).sort(),
		};
	}, [paths]);

	const busiestPaths = useMemo(
		() =>
			[...paths]
				.sort((a, b) => b.readerCount - a.readerCount)
				.slice(0, 4),
		[paths]
	);

	const statusColor =
		summary.total === 0
			? '#9e9e9e'
			: summary.ready === summary.total
			? '#4caf50'
			: summary.ready === 0
			? '#f44336'
			: '#ff9800';

	const updatedLabel = lastUpdated
		? `${t('stream_status_updated')}: ${new Date(lastUpdated).toLocaleTimeString()}`
		: t('stream_status_not_loaded');

	const getQualityColor = (quality: string) => {
		switch (quality) {
			case 'excellent': return '#4caf50';
			case 'good': return '#8bc34a';
			case 'fair': return '#ff9800';
			case 'poor': return '#f44336';
			default: return '#9e9e9e';
		}
	};

	const formatQualityLabel = (quality: string) => {
		const labels: Record<string, string> = {
			excellent: t('webrtc_stats.quality_excellent'),
			good: t('webrtc_stats.quality_good'),
			fair: t('webrtc_stats.quality_fair'),
			poor: t('webrtc_stats.quality_poor'),
			unknown: t('webrtc_stats.quality_unknown'),
		};
		return labels[quality] || quality;
	};

	return (
		<Dialog
			open={open}
			onClose={onClose}
			maxWidth="sm"
			fullWidth
			PaperProps={{
				sx: {
					bgcolor: '#121212',
					color: '#fff',
					borderRadius: 2,
					border: '1px solid rgba(255,255,255,0.08)',
				},
			}}
		>
			<DialogTitle sx={{ pb: 1 }}>
				<Stack direction="row" justifyContent="space-between" alignItems="center" spacing={1.5}>
					<Stack direction="row" spacing={1} alignItems="center">
						<Box
							sx={{
								width: 12,
								height: 12,
								borderRadius: '50%',
								bgcolor: statusColor,
								boxShadow: `0 0 8px ${statusColor}`,
							}}
						/>
						<Typography variant="subtitle1" sx={{ fontWeight: 600 }}>
							{t('stream_status')}
						</Typography>
					</Stack>
					<Stack direction="row" spacing={0.5} alignItems="center">
						<Tooltip title={t('stream_status_refresh')}>
							<span>
								<IconButton
									size="small"
									onClick={() => void loadStatuses()}
									disabled={isRefreshing}
									sx={{ color: '#fff' }}
								>
									<RefreshRoundedIcon fontSize="small" />
								</IconButton>
							</span>
						</Tooltip>
						<Tooltip title={t('close')}>
							<IconButton size="small" onClick={onClose} sx={{ color: '#fff' }}>
								<CloseRoundedIcon fontSize="small" />
							</IconButton>
						</Tooltip>
					</Stack>
				</Stack>
				
				{/* Tabs for switching between Stream Status and WebRTC Stats */}
				<Tabs 
					value={currentTab} 
					onChange={(_, newValue) => setCurrentTab(newValue)}
					sx={{ 
						mt: 1,
						borderBottom: 1,
						borderColor: 'rgba(255,255,255,0.12)',
						'& .MuiTab-root': { 
							color: 'rgba(255,255,255,0.7)',
							minHeight: 40,
						},
						'& .Mui-selected': { 
							color: '#fff !important',
						},
					}}
				>
					<Tab label={t('stream_status')} />
					<Tab 
						label={`WebRTC (${webrtcStatsData.length})`}
						icon={<SignalCellularAltIcon fontSize="small" />}
						iconPosition="start"
					/>
				</Tabs>
			</DialogTitle>

			<DialogContent dividers sx={{ borderColor: 'rgba(255,255,255,0.08)' }}>
				{/* Tab 0: Stream Status (go2rtc) */}
				{currentTab === 0 && (
					<Stack spacing={2}>
						<Stack direction="row" spacing={2}>
							<Stack spacing={0.5} flex={1}>
								<Typography variant="caption" sx={{ opacity: 0.7 }}>
									{t('stream_status_streams')}
								</Typography>
								<Typography variant="body2" sx={{ fontWeight: 600 }}>
									{summary.ready}/{summary.total || '0'} {t('stream_status_ready')}
								</Typography>
							</Stack>
							<Stack spacing={0.5} flex={1}>
								<Typography variant="caption" sx={{ opacity: 0.7 }}>
									{t('stream_status_subscribers')}
								</Typography>
								<Typography variant="body2" sx={{ fontWeight: 600 }}>
									{summary.readers}
								</Typography>
							</Stack>
						</Stack>

						{summary.protocols.length > 0 && (
							<Stack direction="row" spacing={0.5} flexWrap="wrap">
								{summary.protocols.map(protocol => (
									<Chip
										key={protocol}
										label={protocol}
										size="small"
										sx={{ bgcolor: 'rgba(255,255,255,0.12)', color: '#fff' }}
									/>
								))}
							</Stack>
						)}

						{busiestPaths.length > 0 ? (
							<Box>
								<Divider sx={{ borderColor: 'rgba(255,255,255,0.12)', mb: 1 }} />
								<Stack spacing={1.25}>
									{busiestPaths.map(path => (
										<Stack key={path.name} direction="row" alignItems="center" spacing={1}>
											{path.ready ? (
												<CheckCircleRoundedIcon fontSize="small" sx={{ color: '#81c784' }} />
											) : (
												<WarningAmberRoundedIcon fontSize="small" sx={{ color: '#ffb74d' }} />
											)}
											<Box sx={{ flex: 1, overflow: 'hidden' }}>
												<Typography variant="body2" noWrap title={path.name} sx={{ fontWeight: 500 }}>
													{path.name}
												</Typography>
												<Typography variant="caption" sx={{ opacity: 0.7 }}>
													{t('stream_status_connections', { count: path.readerCount })}
												</Typography>
											</Box>
										</Stack>
									))}
								</Stack>
							</Box>
						) : (
							<Typography variant="body2" sx={{ opacity: 0.6 }}>
								{t('stream_status_no_paths')}
							</Typography>
						)}

						<Stack direction="row" justifyContent="space-between" alignItems="center">
							<Typography variant="caption" sx={{ opacity: 0.6 }}>
								{updatedLabel}
							</Typography>
							{error && (
								<Typography variant="caption" color="error" sx={{ fontWeight: 600 }}>
									{error}
								</Typography>
							)}
						</Stack>
					</Stack>
				)}

				{/* Tab 1: WebRTC Statistics */}
				{currentTab === 1 && (
					<Stack spacing={2}>
						{webrtcStatsData.length === 0 ? (
							<Typography variant="body2" sx={{ opacity: 0.6, textAlign: 'center', py: 4 }}>
								Нет активных WebRTC соединений
							</Typography>
						) : (
							webrtcStatsData.map((item, index) => {
								const stats = item.stats;
								const qualityColor = getQualityColor(item.quality);
								
								// Debug logging
								console.log(`[AppStatus] Rendering stats for ${item.streamName}:`, {
									hasVideo: !!stats.video,
									hasAudio: !!stats.audio,
									hasNetwork: !!stats.network,
									connectionState: stats.connectionState,
									iceConnectionState: stats.iceConnectionState,
									stats: stats
								});
								
								return (
									<Box 
										key={`${item.streamName}-${index}`}
										sx={{ 
											p: 1.5, 
											borderRadius: 1,
											border: '1px solid rgba(255,255,255,0.12)',
											bgcolor: 'rgba(255,255,255,0.03)',
										}}
									>
										{/* Stream name and quality */}
										<Stack direction="row" justifyContent="space-between" alignItems="center" mb={1}>
											<Typography variant="body2" sx={{ fontWeight: 600 }}>
												{item.streamName}
											</Typography>
											<Chip 
												label={formatQualityLabel(item.quality)}
												size="small"
												sx={{ 
													bgcolor: `${qualityColor}20`,
													color: qualityColor,
													fontWeight: 600,
													fontSize: '0.7rem',
												}}
											/>
										</Stack>

										{/* Connection state */}
										<Stack spacing={0.5} mb={1}>
											<Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
												<Typography variant="caption" sx={{ opacity: 0.7 }}>
													Connection:
												</Typography>
												<Typography variant="caption" sx={{ fontWeight: 500 }}>
													{stats.connectionState}
												</Typography>
											</Box>
											<Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
												<Typography variant="caption" sx={{ opacity: 0.7 }}>
													ICE:
												</Typography>
												<Typography variant="caption" sx={{ fontWeight: 500 }}>
													{stats.iceConnectionState}
												</Typography>
											</Box>
										</Stack>

										<Divider sx={{ borderColor: 'rgba(255,255,255,0.12)', my: 1 }} />

										{/* Video stats */}
										{stats.video && (
											<Stack spacing={0.5} mb={1}>
												<Typography variant="caption" sx={{ fontWeight: 600, opacity: 0.9 }}>
													📹 {t('webrtc_stats.video')}
												</Typography>
												<Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
													<Typography variant="caption" sx={{ opacity: 0.7 }}>
														{t('webrtc_stats.codec')}:
													</Typography>
													<Typography variant="caption" sx={{ fontWeight: 500 }}>
														{stats.video.codec.toUpperCase()} {stats.video.resolution.width}x{stats.video.resolution.height}
													</Typography>
												</Box>
												<Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
													<Typography variant="caption" sx={{ opacity: 0.7 }}>
														{t('webrtc_stats.bitrate')} / FPS:
													</Typography>
													<Typography 
														variant="caption" 
														sx={{ 
															fontWeight: 500,
															color: stats.video.bitrate > 1000 ? '#4caf50' : stats.video.bitrate > 500 ? '#ff9800' : '#f44336'
														}}
													>
														{stats.video.bitrate} kbps / {stats.video.frameRate} fps
													</Typography>
												</Box>
												<Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
													<Typography variant="caption" sx={{ opacity: 0.7 }}>
														{t('webrtc_stats.packet_loss')} / Jitter:
													</Typography>
													<Typography 
														variant="caption" 
														sx={{ 
															fontWeight: 500,
															color: stats.video.packetLossRate < 1 ? '#4caf50' : stats.video.packetLossRate < 3 ? '#ff9800' : '#f44336'
														}}
													>
														{stats.video.packetLossRate}% / {stats.video.jitter}ms
													</Typography>
												</Box>
												{stats.video.frameDropRate > 0 && (
													<Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
														<Typography variant="caption" sx={{ opacity: 0.7 }}>
															{t('webrtc_stats.frame_drop_rate')}:
														</Typography>
														<Typography 
															variant="caption" 
															sx={{ 
																fontWeight: 500,
																color: stats.video.frameDropRate < 1 ? '#4caf50' : stats.video.frameDropRate < 5 ? '#ff9800' : '#f44336'
															}}
														>
															{stats.video.frameDropRate}%
														</Typography>
													</Box>
												)}
											</Stack>
										)}

										{/* Audio stats */}
										{stats.audio && (
											<Stack spacing={0.5} mb={1}>
												<Typography variant="caption" sx={{ fontWeight: 600, opacity: 0.9 }}>
													🔊 {t('webrtc_stats.audio')}
												</Typography>
												<Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
													<Typography variant="caption" sx={{ opacity: 0.7 }}>
														{t('webrtc_stats.codec')}:
													</Typography>
													<Typography variant="caption" sx={{ fontWeight: 500 }}>
														{stats.audio.codec.toUpperCase()} {stats.audio.bitrate} kbps
													</Typography>
												</Box>
												<Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
													<Typography variant="caption" sx={{ opacity: 0.7 }}>
														{t('webrtc_stats.packet_loss')}:
													</Typography>
													<Typography 
														variant="caption" 
														sx={{ 
															fontWeight: 500,
															color: stats.audio.packetLossRate < 1 ? '#4caf50' : stats.audio.packetLossRate < 3 ? '#ff9800' : '#f44336'
														}}
													>
														{stats.audio.packetLossRate}%
													</Typography>
												</Box>
											</Stack>
										)}

										{/* Network stats */}
										{stats.network && (
											<Stack spacing={0.5}>
												<Typography variant="caption" sx={{ fontWeight: 600, opacity: 0.9 }}>
													🌐 {t('webrtc_stats.network')}
												</Typography>
												<Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
													<Typography variant="caption" sx={{ opacity: 0.7 }}>
														RTT:
													</Typography>
													<Typography 
														variant="caption" 
														sx={{ 
															fontWeight: 500,
															color: stats.network.currentRoundTripTime < 50 ? '#4caf50' : stats.network.currentRoundTripTime < 100 ? '#ff9800' : '#f44336'
														}}
													>
														{stats.network.currentRoundTripTime}ms
													</Typography>
												</Box>
												<Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
													<Typography variant="caption" sx={{ opacity: 0.7 }}>
														{t('webrtc_stats.protocol')}:
													</Typography>
													<Typography variant="caption" sx={{ fontWeight: 500 }}>
														{stats.network.protocol.toUpperCase()}
													</Typography>
												</Box>
												<Box sx={{ display: 'flex', justifyContent: 'space-between' }}>
													<Typography variant="caption" sx={{ opacity: 0.7 }}>
														Candidate:
													</Typography>
													<Typography variant="caption" sx={{ fontWeight: 500 }}>
														{stats.network.localCandidateType} → {stats.network.remoteCandidateType}
													</Typography>
												</Box>
											</Stack>
										)}
									</Box>
								);
							})
						)}
					</Stack>
				)}
			</DialogContent>
		</Dialog>
	);
};

export default AppStatus;