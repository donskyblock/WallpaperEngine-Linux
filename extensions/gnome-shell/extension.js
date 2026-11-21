// SPDX-License-Identifier: GPL-3.0-or-later
//
// Adopts wallpaper windows created by wpe-wallpaper-host / linux-wallpaperengine and moves their
// actors into Shell's background group, so they sit under the desktop, never appear in the overview,
// Alt-Tab or the dock, and follow every workspace.
//
// STATUS: written against the GNOME 45+ extension API from the same approach other video-wallpaper
// extensions use; it has NOT been run on a real GNOME session yet. Please test and report.
import Meta from 'gi://Meta';
import GLib from 'gi://GLib';
import {Extension} from 'resource:///org/gnome/shell/extensions/extension.js';
import * as Main from 'resource:///org/gnome/shell/ui/main.js';

const TITLE_PREFIX = 'wpe-wallpaper:';
const WM_CLASSES = ['org.wpelinux.wallpaperhost', 'linux-wallpaperengine'];

function isWallpaperWindow(win) {
    const title = win.get_title() ?? '';
    const cls = (win.get_wm_class() ?? '').toLowerCase();
    const app = (win.get_gtk_application_id() ?? '').toLowerCase();
    return title.startsWith(TITLE_PREFIX) || WM_CLASSES.includes(cls) || WM_CLASSES.includes(app);
}

export default class WpeLinuxExtension extends Extension {
    enable() {
        this._adopted = new Map();
        this._createdId = global.display.connect('window-created', (_d, win) => this._maybeAdopt(win));
        for (const actor of global.get_window_actors())
            this._maybeAdopt(actor.meta_window);
    }

    disable() {
        if (this._createdId) global.display.disconnect(this._createdId);
        this._createdId = 0;
        for (const [win, info] of this._adopted) this._release(win, info);
        this._adopted.clear();
        this._adopted = null;
    }

    _maybeAdopt(win) {
        if (this._adopted.has(win)) return;
        // Title/class may not be set yet when the window is created, so wait for first frame.
        const id = win.connect('shown', () => {
            win.disconnect(id);
            if (isWallpaperWindow(win)) this._adopt(win);
        });
        if (win.get_compositor_private()?.visible && isWallpaperWindow(win)) {
            win.disconnect(id);
            this._adopt(win);
        }
    }

    _adopt(win) {
        if (this._adopted.has(win)) return;
        const actor = win.get_compositor_private();
        if (!actor) return;

        const monitor = this._monitorFor(win);
        const rect = global.display.get_monitor_geometry(monitor);

        win.stick();
        win.move_resize_frame(false, rect.x, rect.y, rect.width, rect.height);

        const parent = actor.get_parent();
        const bgGroup = Main.layoutManager._backgroundGroup;
        const info = {parent, destroyId: 0};
        if (parent && bgGroup) {
            parent.remove_child(actor);
            bgGroup.add_child(actor);
            actor.set_position(rect.x, rect.y);
            actor.set_size(rect.width, rect.height);
        }
        // Keep it out of Alt-Tab / overview: they iterate window actors in the window group.
        win.skip_taskbar = true;
        info.destroyId = win.connect('unmanaged', () => this._adopted?.delete(win));
        this._adopted.set(win, info);
    }

    _release(win, info) {
        try {
            const actor = win.get_compositor_private();
            if (actor && info.parent && actor.get_parent() !== info.parent) {
                actor.get_parent()?.remove_child(actor);
                info.parent.add_child(actor);
            }
            if (info.destroyId) win.disconnect(info.destroyId);
        } catch (_e) { /* window already gone */ }
    }

    // Titles look like "wpe-wallpaper:DP-1@x,y,w,h" (see wpe-wallpaper-host). Match by geometry so we
    // don't depend on connector-name APIs; fall back to wherever the window landed (lwe windows).
    _monitorFor(win) {
        const m = /@(-?\d+),(-?\d+),(\d+),(\d+)$/.exec(win.get_title() ?? '');
        if (m) {
            const [x, y, w, h] = m.slice(1).map(Number);
            const n = global.display.get_n_monitors();
            for (let i = 0; i < n; i++) {
                const r = global.display.get_monitor_geometry(i);
                if (r.x === x && r.y === y && r.width === w && r.height === h) return i;
            }
        }
        return win.get_monitor();
    }
}
