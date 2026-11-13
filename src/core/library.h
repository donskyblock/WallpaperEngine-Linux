// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "types.h"

namespace wpe {

struct Classified {
    Kind kind = Kind::Unknown;
    QString mainPath; // what to hand to the renderer (file for image/video/web, dir for scene)
};

// Looks at the files, not just at project.json's "type" — Workshop metadata lies.
Classified classify(const QString &path, const QString &typeHint = {});

QList<LibraryItem> scanLibrary(const QString &downloadDir);
QString workshopContentDir(const QString &downloadDir); // <dir>/steamapps/workshop/content/431960

} // namespace wpe
