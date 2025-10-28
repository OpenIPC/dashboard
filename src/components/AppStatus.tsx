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
} from '@mui/material';
import RefreshRoundedIcon from '@mui/icons-material/RefreshRounded';
import WarningAmberRoundedIcon from '@mui/icons-material/WarningAmberRounded';
import CheckCircleRoundedIcon from '@mui/icons-material/CheckCircleRounded';
import CloseRoundedIcon from '@mui/icons-material/CloseRounded';
import type { StreamPathStatus } from '../types';
import { fetchStreamPathStatuses } from '../services/streamBridge';
import { useLocalization } from '../contexts/LocalizationContext';

interface AppStatusProps {
	open: boolean;
	onClose: () => void;
}

const REFRESH_INTERVAL_MS = 15_000;

const AppStatus: React.FC<AppStatusProps> = ({ open, onClose }) => {
	const { t } = useLocalization();
	const [paths, setPaths] = useState<StreamPathStatus[]>([]);
	const [lastUpdated, setLastUpdated] = useState<number | null>(null);
	const [isRefreshing, setIsRefreshing] = useState(false);
	const [error, setError] = useState<string | null>(null);

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
			</DialogTitle>

			<DialogContent dividers sx={{ borderColor: 'rgba(255,255,255,0.08)' }}>
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
			</DialogContent>
		</Dialog>
	);
};

export default AppStatus;