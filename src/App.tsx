import { useEffect, useState } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { useAppStore } from './store/app-store';
import Sidebar from './components/Sidebar';
import BottomBar from './components/BottomBar';
import Toast from './components/Toast';
import Onboarding from './components/Onboarding';
import WorkshopBrowser from './components/workshop/WorkshopBrowser';
import Library from './components/library/Library';
import Settings from './components/settings/Settings';
import './index.css';

export default function App() {
  const { view, loadSettings, loadDisplays, checkTools } = useAppStore();
  const [showOnboarding, setShowOnboarding] = useState(false);

  useEffect(() => {
    (async () => {
      await loadSettings();
      await loadDisplays();
      await checkTools();
      const needsOnboarding = await invoke<boolean>('needs_onboarding');
      if (needsOnboarding) setShowOnboarding(true);
    })();
  }, []);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', height: '100vh', background: 'var(--bg-base)' }}>
      <div style={{ display: 'flex', flex: 1, overflow: 'hidden' }}>
        <Sidebar />
        <main style={{ flex: 1, overflow: 'hidden', display: 'flex', flexDirection: 'column' }}>
          {view === 'workshop' && <WorkshopBrowser />}
          {view === 'library' && <Library />}
          {view === 'settings' && <Settings />}
        </main>
      </div>
      <BottomBar />
      <Toast />
      {showOnboarding && <Onboarding onFinish={() => setShowOnboarding(false)} />}
    </div>
  );
}
