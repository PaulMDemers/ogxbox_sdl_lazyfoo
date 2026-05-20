#pragma once

typedef struct LazyFooLessonInfo {
    int lesson;
    const char *slug;
    const char *title;
} LazyFooLessonInfo;

const LazyFooLessonInfo *lazyfoo_get_lesson_info(int lesson);
