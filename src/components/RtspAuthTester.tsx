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
    </div>
  );
};

export default RtspAuthTester;