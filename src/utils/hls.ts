export interface HlsErrorData {
  type?: string;
  fatal?: boolean;
}

export const isHlsErrorData = (value: unknown): value is HlsErrorData => {
  if (!value || typeof value !== 'object') {
    return false;
  }

  const candidate = value as Partial<HlsErrorData>;
  return typeof candidate.fatal === 'boolean' || candidate.fatal === undefined;
};
