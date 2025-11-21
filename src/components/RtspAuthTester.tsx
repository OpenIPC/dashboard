// Тестирование функциональности аутентификации RTSP
import React, { useEffect, useState } from 'react';
import { invoke } from '@tauri-apps/api/core';

interface TestCase {
  name: string;
  url: string;
  expected: string;
}

interface TestResult extends TestCase {
  result?: string;
  success: boolean;
  error?: string;
}

interface HevcParameterSets {
  vps: string[];
  sps: string[];
  pps: string[];
}

interface HevcProbeResponse {
  rtsp_url: string;
  width: number;
  height: number;
  fps?: number;
  parameter_sets: HevcParameterSets;
  annexb_header: string;
  handshake_log: string[];
}

// Тестовые сценарии для аутентификации RTSP
const TEST_SCENARIOS: TestCase[] = [
  {
    name: "Стандартный URL без специальных символов",
    url: "rtsp://admin:password@192.168.1.100:554/stream=0",
    expected: "rtsp://admin:password@192.168.1.100:554/stream=0"
  },
  {
    name: "URL с @ в имени пользователя",
    url: "rtsp://user@name:password@192.168.1.100:554/stream=0",
    expected: "rtsp://user%40name:password@192.168.1.100:554/stream=0"
  },
  {
    name: "URL с / в пароле",
    url: "rtsp://admin:pass/word@192.168.1.100:554/stream=0",
    expected: "rtsp://admin:pass%2Fword@192.168.1.100:554/stream=0"
  },
  {
    name: "URL с пробелом в пароле",
    url: "rtsp://admin:pass word@192.168.1.100:554/stream=0",
    expected: "rtsp://admin:pass%20word@192.168.1.100:554/stream=0"
  },
  {
    name: "URL с двойным @",
    url: "rtsp://admin:pass@word@192.168.1.100:554/stream=0",
    expected: "rtsp://admin:pass%40word@192.168.1.100:554/stream=0"
  },
  {
    name: "URL со спец. символами в имени пользователя и пароле",
    url: "rtsp://user@name:pass/word@192.168.1.100:554/stream=0",
    expected: "rtsp://user%40name:pass%2Fword@192.168.1.100:554/stream=0"
  },
  {
    name: "Реальный случай с @ в пароле",
    url: "rtsp://admin:USSKot125@192.168.3.44:554/stream=0",
    expected: "rtsp://admin:USSKot125%40@192.168.3.44:554/stream=0"
  }
];

const RtspAuthTester: React.FC = () => {
  const [results, setResults] = useState<TestResult[]>([]);
  const [loading, setLoading] = useState<boolean>(true);
  const [summary, setSummary] = useState<{total: number, passed: number, failed: number}>({
    total: 0, passed: 0, failed: 0
  });
  const [probePath, setProbePath] = useState<string>('');
  const [probeLoading, setProbeLoading] = useState<boolean>(false);
  const [probeError, setProbeError] = useState<string | null>(null);
  const [probeResult, setProbeResult] = useState<HevcProbeResponse | null>(null);

  useEffect(() => {
    const runTests = async () => {
      console.log("=== Тестирование аутентификации RTSP ===");
      
      const testResults: TestResult[] = [];
      
      for (const test of TEST_SCENARIOS) {
        console.log(`\nТест: ${test.name}`);
        console.log(`Исходный URL: ${maskPassword(test.url)}`);
        
        try {
          const result = await invoke('play_direct_rtsp', { sdp: test.url }) as string;
          console.log(`Результат: ${maskPassword(result)}`);
          
          const success = result === test.expected;
          
          if (success) {
            console.log("✅ Тест ПРОЙДЕН");
          } else {
            console.log("❌ Тест НЕ ПРОЙДЕН");
            console.log(`Ожидалось: ${maskPassword(test.expected)}`);
          }
          
          testResults.push({
            ...test,
            result,
            success
          });
        } catch (error) {
          console.error(`❌ Ошибка: ${error}`);
          testResults.push({
            ...test,
            success: false,
            error: String(error)
          });
        }
      }
      
      console.log("\n=== Тестирование завершено ===");
      
      const passed = testResults.filter(t => t.success).length;
      const failed = testResults.length - passed;
      
      setResults(testResults);
      setSummary({
        total: testResults.length,
        passed,
        failed
      });
      setLoading(false);
    };
    
    runTests();
  }, []);
  
  // Функция для маскировки пароля в URL при выводе в лог
  const maskPassword = (url: string): string => {
    const regex = /:([^:@]+)@/;
    return url.replace(regex, ':****@');
  };

  const handleProbe = async () => {
    const trimmed = probePath.trim();
    if (!trimmed) {
      setProbeError('Введите путь к потоку или RTSP URL.');
      setProbeResult(null);
      return;
    }

    setProbeLoading(true);
    setProbeError(null);
    setProbeResult(null);

    try {
      const response = await invoke<HevcProbeResponse>('probe_hevc_export', {
        streamPath: trimmed
      });
      setProbeResult(response);
    } catch (error) {
      const message = typeof error === 'string'
        ? error
        : error instanceof Error
          ? error.message
          : 'Не удалось выполнить проверку.';
      setProbeError(message);
    } finally {
      setProbeLoading(false);
    }
  };
  
  return (
    <div style={{ padding: '20px', maxWidth: '900px', margin: '0 auto' }}>
      <h2 style={{ marginBottom: '20px' }}>Тестирование RTSP аутентификации</h2>
      
      {loading ? (
        <div style={{ padding: '20px', textAlign: 'center' }}>
          <p>Выполняются тесты...</p>
          <div style={{ width: '100%', height: '4px', backgroundColor: '#eee', marginTop: '10px' }}>
            <div style={{ width: '30%', height: '100%', backgroundColor: '#2196F3', animation: 'progress 1s infinite linear' }}></div>
          </div>
        </div>
      ) : (
        <>
          <div style={{ 
            padding: '15px', 
            marginBottom: '20px', 
            backgroundColor: '#f5f5f5',
            borderRadius: '4px',
            display: 'flex',
            justifyContent: 'space-around'
          }}>
            <div><strong>Всего тестов:</strong> {summary.total}</div>
            <div style={{ color: 'green' }}><strong>Пройдено:</strong> {summary.passed}</div>
            <div style={{ color: 'red' }}><strong>Не пройдено:</strong> {summary.failed}</div>
          </div>
          
          <div>
            {results.map((result, index) => (
              <div key={index} style={{ 
                border: '1px solid #ddd', 
                borderRadius: '4px',
                marginBottom: '15px',
                overflow: 'hidden'
              }}>
                <div style={{ 
                  padding: '10px 15px', 
                  backgroundColor: result.success ? '#e8f5e9' : '#ffebee',
                  borderBottom: '1px solid #ddd',
                  display: 'flex',
                  alignItems: 'center'
                }}>
                  {result.success ? (
                    <span style={{ color: 'green', marginRight: '8px' }}>✅</span>
                  ) : (
                    <span style={{ color: 'red', marginRight: '8px' }}>❌</span>
                  )}
                  <h3 style={{ margin: 0, fontSize: '16px' }}>{result.name}</h3>
                </div>
                
                <div style={{ padding: '15px' }}>
                  <div style={{ marginBottom: '10px' }}>
                    <div><strong>Исходный URL:</strong></div>
                    <div style={{ 
                      backgroundColor: '#f5f5f5', 
                      padding: '8px', 
                      borderRadius: '4px',
                      fontFamily: 'monospace',
                      overflowX: 'auto',
                      fontSize: '14px'
                    }}>
                      {maskPassword(result.url)}
                    </div>
                  </div>
                  
                  {result.result && (
                    <div style={{ marginBottom: '10px' }}>
                      <div><strong>Результат:</strong></div>
                      <div style={{ 
                        backgroundColor: '#f5f5f5', 
                        padding: '8px', 
                        borderRadius: '4px',
                        fontFamily: 'monospace',
                        overflowX: 'auto',
                        fontSize: '14px'
                      }}>
                        {maskPassword(result.result)}
                      </div>
                    </div>
                  )}
                  
                  {!result.success && (
                    <div style={{ marginBottom: '10px' }}>
                      <div><strong>Ожидалось:</strong></div>
                      <div style={{ 
                        backgroundColor: '#f5f5f5', 
                        padding: '8px', 
                        borderRadius: '4px',
                        fontFamily: 'monospace',
                        overflowX: 'auto',
                        fontSize: '14px',
                        color: '#d32f2f'
                      }}>
                        {maskPassword(result.expected)}
                      </div>
                    </div>
                  )}
                  
                  {result.error && (
                    <div style={{ color: 'red', marginTop: '10px' }}>
                      <strong>Ошибка:</strong> {result.error}
                    </div>
                  )}
                </div>
              </div>
            ))}
          </div>
        </>
      )}

      <div style={{ marginTop: '40px' }}>
        <h2 style={{ marginBottom: '12px' }}>Проверка HEVC / H.265 потока</h2>
        <p style={{ marginBottom: '16px', color: '#555' }}>
          Введите название потока из go2rtc или полный RTSP URL, чтобы выполнить RTSP рукопожатие и получить параметры HEVC потока.
        </p>

        <div style={{ display: 'flex', gap: '10px', marginBottom: '12px', flexWrap: 'wrap' }}>
          <input
            type="text"
            placeholder="cam1_main или rtsp://user:pass@host:554/stream"
            value={probePath}
            onChange={(event) => setProbePath(event.target.value)}
            style={{
              flex: '1 1 300px',
              padding: '10px',
              borderRadius: '4px',
              border: '1px solid #ccc',
              fontSize: '14px'
            }}
          />
          <button
            onClick={handleProbe}
            disabled={probeLoading}
            style={{
              padding: '10px 18px',
              borderRadius: '4px',
              border: 'none',
              backgroundColor: probeLoading ? '#9e9e9e' : '#1976d2',
              color: '#fff',
              cursor: probeLoading ? 'not-allowed' : 'pointer',
              fontSize: '14px'
            }}
          >
            {probeLoading ? 'Выполняется...' : 'Проверить поток'}
          </button>
        </div>

        {probeError && (
          <div style={{
            backgroundColor: '#ffebee',
            border: '1px solid #ffcdd2',
            color: '#c62828',
            borderRadius: '4px',
            padding: '12px',
            marginBottom: '16px'
          }}>
            <strong>Ошибка:</strong> {probeError}
            {probeError.includes('disabled') && (
              <div style={{ marginTop: '8px', color: '#ad1457' }}>
                Соберите приложение с фичей <code>hevc-export</code> и установленным LLVM/Clang, чтобы активировать экспорт HEVC.
              </div>
            )}
          </div>
        )}

        {probeResult && (
          <div style={{
            border: '1px solid #cfd8dc',
            borderRadius: '4px',
            padding: '16px',
            backgroundColor: '#f5f9ff'
          }}>
            <div style={{ marginBottom: '12px' }}>
              <strong>RTSP URL:</strong>
              <div style={{
                marginTop: '4px',
                backgroundColor: '#fff',
                borderRadius: '4px',
                border: '1px solid #e0e0e0',
                padding: '8px',
                fontFamily: 'monospace',
                fontSize: '13px',
                wordBreak: 'break-all'
              }}>
                {maskPassword(probeResult.rtsp_url)}
              </div>
            </div>

            <div style={{ display: 'flex', flexWrap: 'wrap', gap: '16px', marginBottom: '16px' }}>
              <div><strong>Разрешение:</strong> {probeResult.width}×{probeResult.height}</div>
              {typeof probeResult.fps === 'number' && (
                <div><strong>FPS:</strong> {probeResult.fps.toFixed(2)}</div>
              )}
            </div>

            <div style={{ marginBottom: '16px' }}>
              <strong>Annex-B заголовок (Base64):</strong>
              <div style={{
                marginTop: '4px',
                backgroundColor: '#fff',
                borderRadius: '4px',
                border: '1px solid #e0e0e0',
                padding: '8px',
                fontFamily: 'monospace',
                fontSize: '13px',
                wordBreak: 'break-all'
              }}>
                {probeResult.annexb_header}
              </div>
            </div>

            <div style={{ marginBottom: '16px' }}>
              <strong>Parameter Sets (Base64):</strong>
              <div style={{
                marginTop: '8px',
                display: 'grid',
                gap: '8px'
              }}>
                {['vps', 'sps', 'pps'].map((key) => {
                  const values = probeResult.parameter_sets[key as keyof HevcParameterSets];
                  return (
                    <div key={key}>
                      <div style={{ fontWeight: 600, marginBottom: '4px' }}>{key.toUpperCase()}:</div>
                      {values.length === 0 ? (
                        <div style={{ fontStyle: 'italic', color: '#607d8b' }}>нет данных</div>
                      ) : (
                        values.map((item, idx) => (
                          <div
                            key={`${key}-${idx}`}
                            style={{
                              backgroundColor: '#fff',
                              borderRadius: '4px',
                              border: '1px solid #e0e0e0',
                              padding: '8px',
                              fontFamily: 'monospace',
                              fontSize: '13px',
                              wordBreak: 'break-all',
                              marginBottom: idx === values.length - 1 ? 0 : '6px'
                            }}
                          >
                            {item}
                          </div>
                        ))
                      )}
                    </div>
                  );
                })}
              </div>
            </div>

            <div>
              <strong>Лог RTSP рукопожатия:</strong>
              <div style={{
                marginTop: '8px',
                backgroundColor: '#fff',
                borderRadius: '4px',
                border: '1px solid #e0e0e0',
                padding: '12px',
                maxHeight: '200px',
                overflowY: 'auto',
                fontFamily: 'monospace',
                fontSize: '13px'
              }}>
                {probeResult.handshake_log.length === 0 ? (
                  <div style={{ fontStyle: 'italic', color: '#607d8b' }}>Журнал пуст.</div>
                ) : (
                  probeResult.handshake_log.map((line, idx) => (
                    <div key={idx} style={{ marginBottom: '4px', whiteSpace: 'pre-wrap' }}>{line}</div>
                  ))
                )}
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default RtspAuthTester;