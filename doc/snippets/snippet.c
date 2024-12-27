char buffer[20] = "";
    struct tm *t;
    
    t = localtime(&time_local);
    strftime(buffer, 20, "%H:%M", t);
    lcd_Printf("%s\n", buffer);
    
    t = localtime(&date_local);
    strftime(buffer, 20, "%d/%m/%y", t);
    lcd_Printf("%s\n", buffer);

